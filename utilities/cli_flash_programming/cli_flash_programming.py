#########################################################################################
# Copyright (C) 2019 Dialog Semiconductor.
# This computer program includes Confidential, Proprietary Information
# of Dialog Semiconductor. All Rights Reserved.
#########################################################################################

import argparse
import atexit
import re

from cli import *
from message import *

# CLI_VERSION_MAJOR = 1
# CLI_VERSION_MINOR = 0
status_code = False


@atexit.register
def goodbye():
    if status_code:
        print('CLI Success')
    else:
        print('CLI Failed')


def retrieve_version(mm):
    status, error, ver = mm.send_and_recv(RequestOpCode.ACTION_READ_VERSION)
    validate_response("ACTION_READ_VERSION", status, error)
    logging.info("Target's flash_programmer version: %s" % ver.decode())


def validate_response(command, status, error, data=None):
    if error:
        try:
            error_code = ResponseStatus(status).name
        except:
            error_code = "unknown"
        raise Exception("Command %s returned with %s error code." % (command, error_code))
    else:
        logging.info("Command %s returned successfully." % command)


def setup_spi_pins(mm, device, spi_cfg):
    if spi_cfg is None:
        if device == "DA14531":
            spi_cfg = "0,1,0,4,0,0,0,3"
        elif device == "DA14585":
            spi_cfg = "0,3,0,0,0,6,0,5"

    cs_port, cs_pin, clk_port, clk_pin, do_port, do_pin, di_port, di_pin = spi_cfg.split(',')
    spi_data = pack('<BBBBBBBBB',
                    int(cs_port.strip()), int(cs_pin.strip()),
                    int(clk_port.strip()), int(clk_pin.strip()),
                    int(do_port.strip()), int(do_pin.strip()),
                    int(di_port.strip()), int(di_pin.strip()),
                    0)

    status, error, resData = mm.send_and_recv(RequestOpCode.ACTION_SPI_GPIOS, spi_data)
    validate_response("ACTION_SPI_GPIOS", status, error, resData)


def add_bootable_image_header(bin_file_bytes):
    # Bootable image 70 50 00 00 00 00 <2 last bytes image size without this header>
    bootable_header = pack('>BBBBBBH',
                           int("70", 16), int("50", 16),
                           int("00", 16), int("00", 16),
                           int("00", 16), int("00", 16),
                           len(bin_file_bytes))
    return bootable_header + bin_file_bytes


def validate_args(args):
    if args.port and 'COM' not in args.port:
        raise Exception("--port argument is invalid.")
    if args.device and args.device != "DA14531":
        raise Exception("--device argument is invalid. Supported device is DA14531.")
    if args.firmware:
        regexp = re.compile(r'flash_programmer')
        if not regexp.search(args.firmware.name):
            raise Exception("--firmware argument is invalid.")
    if args.spi_cfg:
        if not args.jlink:
            spi_cfg = args.spi_cfg.split(",")
            if len(spi_cfg) != 8:
                raise Exception("--spi_cfg argument must have 8 values. eg. PORT_CS,PIN_CS,PORT_CLK,PIN_CLK,PORT_DO,PIN_DO,PORT_DI,PIN_DI")
            for p in spi_cfg:
                if not p.isdigit():
                    raise Exception("--spi_cfg, all arguments must be integers. eg. 0,1,0,4,0,0,0,3")
        else:
            raise Exception("--spi_cfg argument is valid only for --uart option.")
    if args.serialno and len(args.serialno) != 9:
        raise Exception("--serialno argument is invalid. serialno must have 9 digits.")
    if args.port_selector is not None and not args.jlink:
        if args.device == "DA14531" and args.port_selector != 5:
            raise Exception("The only valid UART port selector for DA14531 targets is 5 (single wire UART at P0_5 pin).")


def do_erase(args):
    if args.jlink:
        return jlink_erase(args.device, args.serialno, args.start, args.size)
    else:
        mm = MessageManager(port=args.port, device=args.device, selector=args.port_selector, userUartFirmware=args.firmware)
        # retrieve_version(mm)
        setup_spi_pins(mm, args.device, args.spi_cfg)
        if args.start == 0 and args.size == -1:
            status, error, respData = mm.send_and_recv(RequestOpCode.ACTION_SPI_ERASE)
            validate_response("ACTION_SPI_ERASE", status, error, respData)
        elif args.start != 0 and args.size == -1:
            logging.info('invalid erase size')
        else:
            SECTOR_SIZE = 4096
            # size in bytes
            status, error, respData = mm.send_and_recv(
                RequestOpCode.ACTION_SPI_ERASE_BLOCK,
                pack('>IH', args.start, int((args.size-1)/SECTOR_SIZE+1))
            )
            validate_response("ACTION_SPI_ERASE_BLOCK", status, error, respData)
        # do this always! Otherwise you will need unplug device
        mm.send_and_recv(RequestOpCode.ACTION_PLATFORM_RESET)

        return True


def do_read(args):
    if args.jlink:
        return jlink_read_to_file(args.device, args.serialno, args.bin, args.start, args.size)
    else:
        mm = MessageManager(port=args.port, device=args.device, selector=args.port_selector, userUartFirmware=args.firmware)
        # retrieve_version(mm)
        setup_spi_pins(mm, args.device, args.spi_cfg)
        if args.size == -1:
            logging.info('invalid read size')
        elif args.start == -1:
            logging.info('invalid read start')
        else:
            chunk_size = 0x4000
            if args.size < chunk_size:
                chunk_size = args.size
            for chunk in range(0, args.size, chunk_size):
                if chunk + chunk_size > args.size:
                    chunk_size = args.size - chunk
                status, error, respData = mm.send_and_recv(
                    RequestOpCode.ACTION_SPI_READ,
                    pack('>IH', args.start+chunk, chunk_size)
                )
                validate_response("ACTION_SPI_READ", status, error, respData)
                if args.bin is None:
                    print(binascii.hexlify(respData).decode("utf-8"), end='')
                else:
                    args.bin.write(respData)
            print("")
            if args.bin is not None:
                args.bin.close()
        # do this always! Otherwise you will need unplug device
        mm.send_and_recv(RequestOpCode.ACTION_PLATFORM_RESET)

        return True


def do_write(args):
    if args.bin.name == "":
        logging.info('missing input file name')
    if args.size == -1:
        logging.info('invalid write size')
    else:
        if args.size:
            bin = args.bin.read(args.size)
        else:
            bin = args.bin.read()
            if args.bootable:
                bin = add_bootable_image_header(bin)

        if args.jlink:
            result = jlink_write_from_file(args.device, args.serialno, bin, args.start, args.size)
            args.bin.close()
            return result
        else:
            mm = MessageManager(port=args.port, device=args.device, selector=args.port_selector, userUartFirmware=args.firmware)
            # retrieve_version(mm)
            setup_spi_pins(mm, args.device, args.spi_cfg)

            if args.size == 0:
                args.size = len(bin)
            if args.size > len(bin):
                logging.info('invalid write size: %d > %d (binary file)' % (args.size, len(bin)))
            else:
                chunk_size = 0x4000
                for chunk in range(0, args.size, chunk_size):
                    data = bin[chunk:min(chunk + chunk_size, args.size)]
                    status, error, respData = mm.send_and_recv(
                        RequestOpCode.ACTION_SPI_WRITE,
                        pack('>IH', args.start + chunk, len(data)) + data
                    )
                    validate_response("ACTION_SPI_WRITE", status, error, respData)
            args.bin.close()

            # do this always! Otherwise you will need unplug device
            mm.send_and_recv(RequestOpCode.ACTION_PLATFORM_RESET)

        return True


# helper method for argparse that allow enter integer values in both decimal and hexadecimal format
def any_int(x):
    return eval(x)

###################
#       MAIN      #
###################
# print('cli_flash_programming %d.%02d' % (CLI_VERSION_MAJOR, CLI_VERSION_MINOR))
# print('Copyright (c) 2019-2020 Dialog Semiconductor')

# The program has two sets of parameters, that depend on selected link type (uart or jlink)
# and on selected command to issue (write, read, erase)
# Some commands are unavailable for jlink eg. spi_cfg as they are hardcoded in J-Link software,

# MAIN PARSER
parser = argparse.ArgumentParser(description='Tool for programming SPI FLASH',
                                 formatter_class=argparse.RawTextHelpFormatter)
parser.add_argument('--verbose', '-v', help='verbose', action='count')

# GROUP PARSER: link_type
group_link_type = parser.add_mutually_exclusive_group(required=True)

# GROUP PARSER: link_type/uart
group_link_type.add_argument('--uart', help='use uart/rs232 for programming', action='store_true')
group_link_type.add_argument('--jlink', help='use jlink/jtag for programming', action='store_true')

group_selector = parser.add_mutually_exclusive_group()
group_selector.add_argument('--serialno', type=str, default=None, help='''
        Specify serial number of the board to be connected.
        Available only for boards with Segger J-Link module.
        Used only with --jlink option.
        ''')
group_selector.add_argument('--port', type=str, default=None, help='''
        Specify serial port where device is connected. Leave empty for auto-detection.
        Used only with --uart option.
        ''')
parser.add_argument('--firmware', type=argparse.FileType('rb'), help='''
        First stage of boot procedure do not allow flash programming. If required,
        specify dedicated flash_programmer binary file to be loaded.
        If omitted, use one from default location (in the same folder a cli_flash_programming).
        Used only with --uart option.
        ''')
parser.add_argument('--spi_cfg', type=str, help='''
        Define GPIO pin assignment for SPI flash. It may be required on custom boards.
        The configuration is a comma separated list of 4 pair of numbers. 
        Each pair describe port and pin for CS, CLK, DO and DI, 
        eg. --spi_cfg 0,5,0,4,0,6,0,3
        If --spi_cfg argument is not selected, then default SPI Flash pins are used:
        DA14531: --spi_cfg 0,1,0,4,0,0,0,3
        Used only with --uart option.
        ''')
parser.add_argument('--device', type=str, default='DA14531', help='''
        Force to use specific target device. 
        Default value is DA14531.
        ''')
parser.add_argument('--port_selector', type=int, default=5, help='''
        Define GPIO pin assignment for UART. The cli_flash_programming uses by default
        selector 5 through pin P0_5 at speed 115200 for DA14531.''')
parser.add_argument('--bootable', action='store_true', help='''
        Automatically add an AN-B-001 header at offset 0 of the SPI flash memory. 
        The secondary bootloader will copy only the number of bytes defined in the 
        SPI Flash header''')

# SUB PARSER: command
subparser_command = parser.add_subparsers(help='Select command')

# SUB PARSER: command/erase
parser_command = subparser_command.add_parser('erase',
                                              help='erase SPI flash',
                                              formatter_class=argparse.RawTextHelpFormatter)
parser_command.set_defaults(func=do_erase)

parser_command.add_argument('start', type=any_int, nargs='?', default=0, help='''
        Start address to be erased. Default is 0.
        NOTE: If start=0 and size=-1, then command will erase whole SPI flash.
              If start!=0 and size=-1, then command will return error.
        WARNING: if the start address is not aligned to page it will erase whole page.
        ''')
parser_command.add_argument('size', type=any_int, nargs='?', default=-1, help='''
        Size of the area to be erased. Default is -1.
        NOTE: If start=0 and size=-1, then command will erase whole SPI flash.
              If start!=0 and size=-1, then command will return error.
        WARNING: if the start address is not aligned to page it will erase whole pages.
        ''')

# SUB PARSER: command/read
parser_command = subparser_command.add_parser('read',
                                              help='read SPI flash',
                                              formatter_class=argparse.RawTextHelpFormatter)
parser_command.set_defaults(func=do_read)

parser_command.add_argument('start', type=any_int, nargs='?', default=0, help='''
        Start address to be read. Default is 0.
        ''')
parser_command.add_argument('size', type=any_int, nargs='?', default=1, help='''
        Size of the area to be read. Default is 1.
        ''')
parser_command.add_argument('bin', type=argparse.FileType('wb+'), nargs='?', help='''
        Optional output file where to store read data.
        ''')

# SUB PARSER: command/write
parser_command = subparser_command.add_parser('write',
                                              help='write SPI flash',
                                              formatter_class=argparse.RawTextHelpFormatter)
parser_command.set_defaults(func=do_write)

parser_command.add_argument('start', type=any_int, nargs='?', default=0, help='''
        Start address to be written.
        ''')
parser_command.add_argument('size', type=any_int, nargs='?', default=0, help='''
        Size of the area to be written.
        NOTE: if size equal to 0, write whole file to SPI flash.
        ''')
parser_command.add_argument('bin', type=argparse.FileType('rb'), help='''
        Input file from where to read data.
        ''')

if len(sys.argv) == 1:
    parser.print_help(sys.stderr)
    sys.exit(1)
args = parser.parse_args()
validate_args(args)

# set the verbosity
if args.verbose is None or args.verbose == 0:
    logging.basicConfig(level=logging.WARNING)
elif args.verbose > 1:
    logging.basicConfig(level=logging.DEBUG)
elif args.verbose == 1:
    logging.basicConfig(level=logging.INFO)

status_code = args.func(args)

