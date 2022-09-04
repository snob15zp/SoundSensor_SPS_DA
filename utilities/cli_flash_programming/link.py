"""UART link comunication level
Link Manager is an object that establish connection with target
and prepare the target for execution upper level commands like SPI_WRITE
"""

import binascii
import time
from ui import uiSelect
from debug import *
from os import path
from serial import Serial, SerialException
from struct import pack

STX = 0x02
SOH = 0x01
ACK = 0x06
NAK = 0x15

UART_OPTIONS = {0: 57600,
                2: 115200,
                3: 115200,
                4: 57600,
                5: 115200,
                6: 9600}


class LinkManager(Serial):
    def __init__(self, port=None, device=None, selector=None, userUartFirmware=None):
        """Constructor: prepare serial port and UART firmware

        The usual case is that user has connected just one target
        and this function search for the target. In case more are found
        a prompt display with list of allowed selections.

        The firmware could be specified by the user, but if not
        the script will try to load one from well known location.
        Calculate CRC32 for future use.

        :param port: The name of special device of serial port (eg /dev/ttyUSB0 or COM0:).
            If omitted, search for available ports and ask interactively.
        :type port: str

        :param userUartFirmware: The file descriptor of the executable binary file for alternative UART firmware
            known also as flash_programmer. If omitted, try to use one from well known location.
            (default is None)
        :type userUartFirmware: class '_io.BufferedReader'
        """
        self.device = device
        firmware_file = None
        if userUartFirmware:
            firmware_file = userUartFirmware
        else:
            if device == "DA14531":
                defaultPaths = (path.normpath('./flash_programmer_531.bin'), 'flash_programmer_531.bin')
            elif device == "DA14585":
                defaultPaths = (path.normpath('./flash_programmer_585.bin'), 'flash_programmer_585.bin')
            for uartFirmware in defaultPaths:
                if path.isfile(uartFirmware):
                    firmware_file = open(uartFirmware, 'rb')

        if not firmware_file:
            raise OSError("Couldn't find uart firmware:", uartFirmware)
        cli_debug('UART: use %s as UART firmware' % firmware_file.name)
        self.firmware = firmware_file.read()

        # Change UART configuration by appending to the binary 2 bytes at the end of flash_programmer.bin file
        # Add zero bytes for padding
        if self.firmware[len(self.firmware) - 1] != selector:
            padding_bytes = 8 - len(self.firmware) % 8
            for zeros in range(0, padding_bytes):
                self.firmware = self.firmware + pack('B', 0)

            self.firmware = self.firmware + pack('B', int(selector))

        firmware_file.close()

        self.firmware_crc = 0
        for b in self.firmware:
            self.firmware_crc = self.firmware_crc ^ b

        # check if given port is attached to host
        available_uart_ports = self.serialPorts()
        port_found = False
        for uart_port in available_uart_ports:
            if uart_port[1] == port:
                port_found = True
                break

        if not port or not port_found:
            port = self.findSerialPort()
        cli_debug("UART port: %s, baudrate: %s" % (port, UART_OPTIONS[selector]))
        super().__init__(port, baudrate=UART_OPTIONS[selector], exclusive=True)

    def serialPorts(self):
        """ Lists serial port names

            :raises EnvironmentError:
                On unsupported or unknown platforms
            :returns:
                A list of the serial ports available on the system
        """
        ports = ['COM%s' % (i + 1) for i in range(256)]

        result = []
        for port in ports:
            try:
                s = Serial(port)
                s.close()
                result.append(("<>", port, ""))
            except (OSError, SerialException):
                pass
        return result

    def findSerialPortOnOS(self):
        """Find and select serial port.

            This function uses this pattern to prepare a list of available devices.
            If only one is available, then use it without user interaction. If more, then prompt for
            user's decision.

            :return: the COM number of the selected port
            :rtype: str
        """
        uart_ports = self.serialPorts()
        num_ports = len(uart_ports)
        cli_debug('Found %d Serial interfaces' % num_ports)
        if num_ports == 0:
            raise RuntimeError("UART connection error: no target found")
        if num_ports == 1:
            (serial, tty_path, id) = uart_ports[0]
        else:
            selected_serial = uiSelect(uart_ports,
                                       'Select serial port',
                                       'Please select one of the targets from the list')
            if selected_serial < 0:
                raise RuntimeError('UART: Serial port interface selection abort')
            serial, tty_path, id = uart_ports[selected_serial]
        cli_info('Select UART: %s on %s' % (serial, tty_path))
        return tty_path

    def findSerialPort(self):
        return self.findSerialPortOnOS()

    def connect(self):
        """Connect to target

        This method implement initial handshaking protocol described for the UART boot sequence:
        host    <->   target
                <--   STX
        SOH     -->
        len     -->
                <--   ACK
        firmware -->
                <--   CRC8
        ACK     -->
                <--   'Hello'
        """
        cmdMsg = pack('B', 0xC0)
        crc32 = binascii.crc32(cmdMsg + b'') & 0xffffffff
        bufHeadSect = pack('>HI', len(cmdMsg) + len(b''), crc32)
        msg = bufHeadSect + cmdMsg + b''
        self.write(msg)

        cli_debug('UART connection: ++ First stage: boot procedure  ++')
        cli_msg('Please press the hardware reset button on the board to start the download process..')
        prompt_time = time.time() + 4
        start_timeout = time.time()
        while True:
            x = self.read(1, 4)
            if not x:
                cli_msg('Please press hw RESET button on the board!')
                if time.time() > start_timeout + 12:
                    raise OSError("Timeout: Reset signal not detected for more than 12000 msecs.")
                continue
            if x[0] == STX:
                cli_debug('UART connection: received STX, sending SOH')
                buf = pack('<BH', SOH, len(self.firmware))
                self.write(buf)
                x = self.read(1)
                if not x:
                    raise OSError("UART connection error: device didn't respond to initial request")
                cli_debug('UART connection: received respond to SOH: 0x%02x' % x[0])
                if x[0] == NAK:
                    raise OSError("UART connection error: device responded with NAK")
                elif x[0] != ACK:
                    raise OSError("UART, connection error: device send invalid respond: 0x%02X" % x[0])
                else:
                    break
            else:
                cli_debug('UART connection: waiting for STX, read unknown byte: 0x%02X' % x[0])
                if time.time() > prompt_time:
                    prompt_time = time.time() + 4
                    cli_msg('Please RESET the board!')

        self.write(self.firmware, progressbar=True, mesg='Uploading UART firmware file')
        crc = self.read(1, 1)
        if not crc:
            self.write(pack('B', NAK))
            raise OSError("UART connection: target didn't return CRC")
        elif crc[0] != self.firmware_crc:
            self.write(pack('B', NAK))
            raise OSError("UART connection: target return wrong CRC: 0x%02X (expected: 0x%02X)" % (crc[0], self.firmware_crc))
        cli_debug("UART connection: CRC ok")
        self.write(pack('B', ACK))
        cli_debug('UART connection: ++ Second stage: Check flash_programmer successful download ++')
        x = self.read(1)    # TODO: dummy read is required. No explanation!
        x = self.read(5, 4)
        if self.device == "DA14585":
            if not x:
                raise OSError("UART connection error: target didn't get up after loading uartFirmware")
            if x != b'Hello':
                raise OSError("UART connection error: target send invalid sequence. Is the uartFirmware correct?")
        cli_msg('++ UART %s connected successfully ++' % self.device)

    def read(self, length=1, timeout=-1, executionTime=0, clear_line=False):
        """Overloaded read method

        Read data from serial port.

        :param length: number of bytes to send
        :type length: int

        :param timeout: Explicit time in seconds to wait for completion, if -1, calculate timeout
            depend on the baudrate and data length.
        :type timeout: float

        :param executionTime: Some reads may start before the target is ready eg. after issuing SPI_READ cmd.
            in such case, automatically calculated timeout need to be increased by executionTime in seconds.
        :type executionTime: float

        :param clear_line: In case of 1-wire UART, data written on the line should be cleared before read of the command
            reply
        :type clear_line: Boolean

        :return: data read from serial port
        :rtype: bytes
        """
        if clear_line:
            total_bytes_read = bytes()
            while True:
                bytes_to_read = super().inWaiting()
                if bytes_to_read > 0:
                    if len(total_bytes_read) + bytes_to_read > length:
                        bytes_to_read = length - len(total_bytes_read)
                    total_bytes_read = total_bytes_read + super().read(bytes_to_read)
                    if len(total_bytes_read) == length:
                        break
        else:
            if timeout != -1:
                self.timeout = timeout
            else:
                # timeout is 1.5 of time to transmit length of data with baudrate (only 8 of 10 signs are data)
                if not executionTime:
                    executionTime = 0.025
                self.timeout = 2.0 * 10.0 * length / self.baudrate + executionTime

            return super().read(length)

    def write(self, buf, timeout=-1, progressbar=True, mesg='UART sending'):
        """Overloaded write method

        Write data to serial port.

        :param buf: Data to be send
        :type buf: bytes

        :param timeout: Explicit time in seconds to wait for completion, if -1, calculate timeout
            depend on the baudrate and data length.
        :type timeout: float

        :param progressbar: A large transfer take time. If progressbar equal to True and transfer is
            expected to take longer than 0.5s, method will print # for each sent "0.5s" chunk of data.
        :type progressbar: Boolean

        :param mesg: Short message printed before printing a progress bar.
        :type mesg: str
        """
        if timeout != -1:
            self.write_timeout = timeout
        else:
            # timeout is 1.5 of time to transmit length of data with baudrate (only 8 of 10 signs are data)
            self.write_timeout = 10.0 * len(buf) / self.baudrate + 0.02
        chunk_size = int(self.baudrate/10/8)    # calculate chunk of data, that shall be transferred in 0.5s

        if progressbar and (chunk_size*2 < len(buf)):
            cli_msg(mesg, "[", end='', flush=True)
            for index in range(chunk_size, len(buf), chunk_size):
                super().write(buf[index-chunk_size:index])
                if self.device == "DA14531":
                    self.read(chunk_size, clear_line=True)
                cli_msg("#", end='', flush=True)
            super().write(buf[index:])
            if self.device == "DA14531":
                self.read(len(buf)-index, clear_line=True)
            cli_msg("#]")
        else:
            super().write(buf)
            if self.device == "DA14531":
                self.read(len(buf), clear_line=True)
        self.flush()
