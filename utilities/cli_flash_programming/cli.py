#!/usr/bin/env python3
# The MIT License (MIT)
# Copyright (c) 2019 ezflash
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
# IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
# DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
# OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
# OR OTHER DEALINGS IN THE SOFTWARE.


__version__ = "0.0.2"


import tempfile
import collections
import binascii

from ui import uiSelect
from smartbondDevices import *

'''JTAG Manufacture ID of Dialog Semiconductor
This a a constant and is used to filter out non Dialog devices.
'''
JTAG_MANUF_DEF = 0x023B

'''Definitions of supported devices by JTAG IDCODE
Each chip with JTAG interface has an unique JTAG IDCODE.
This information is used to obtain vital information for certain operations.
'''
DEV_DATA = collections.namedtuple('DEV_DATA', ['name', 'offset', 'size', 'aclass'])
DEVICE_DEF = {
    0xbc11: DEV_DATA('DA14531', 0x16000000, 0x00040000, DA14531),
}


class ezFlashCLI():
    def __init__(self, device_name, serialno):
        # # load the flash database
        # with open(os.path.join(os.path.dirname(os.path.abspath(__file__)), 'flash_database.json')) as json_file:
        #     self.flash_db = json.load(json_file)
        #
        #     json_file.close()
        logging.info("By using the program in --jlink mode you accept the SEGGER J-link™ license")

        self.link = Pyjlink()
        rawdevicelist = self.link.browse()
        devicelist = []
        for device in rawdevicelist:
            if device.SerialNumber != 0:
                serialno_local = str(device.SerialNumber)
                mancode, idcode = self.link.identify(serialno_local)
                if mancode != JTAG_MANUF_DEF:
                    continue
                try:
                    device_name_local = DEVICE_DEF[idcode].name
                except KeyError:
                    logging.debug('JLINK: unsupported device, JTAG IDCODE: 0x%x' % idcode)
                    device_name_local = None

                if serialno is not None:
                    if serialno_local == serialno:
                        if device_name_local is None:
                            raise RuntimeError('The device SN %s is connected, but it is unsupported device.'%serialno)
 
                        if device_name is not None and device_name.upper() != device_name_local:
                            raise RuntimeError(
                                'The device SN %s is connected, but its device type do not match requested one. '
                                'Please, correct --device or --serialno option.' % serialno)
                        devicelist.clear()
                        devicelist.append((device_name_local, serialno_local, idcode))
                        break
                elif device_name is None or device_name.upper() == device_name_local:
                    if device_name_local is not None:
                        devicelist.append((device_name_local, serialno_local, idcode))

        device_count = len(devicelist)
        if device_count < 1:
            raise RuntimeError('No devices found')
        elif device_count == 1:
            selected_jlink = devicelist[0]
        else: # device_count > 1:
            index = uiSelect(devicelist, 'Select JLink device', 'Please select one of the devices from the list')
            if index < 0:
                raise RuntimeError('JLINK: JLink interface selection abort')
            selected_jlink = devicelist[index]

        self.device_def = DEVICE_DEF[selected_jlink[2]]
        self.link.select(selected_jlink[1])
        self.link.connect(selected_jlink[0])
        self.jlink_native_support = self.link.Device.startswith('DA14')
        logging.info('Select J-Link: %s on %s' % (self.link.Device, self.link.serialno))
        self.link.reset()

        self.da = self.device_def.aclass(self.link)

    def erase_chip(self):
        if self.jlink_native_support:
            return self.link.erase_chip()

    def loadfile(self, fname, addr):
        if self.jlink_native_support:
            return self.link.loadfile(fname, addr + self.device_def.offset)

    def savefile(self, toFile, addr, size):
        if self.jlink_native_support:
            if size == 0:
                size = self.device_def.size

            read_data = self.da.read_flash(addr, size)
            if toFile:
                toFile.write(bytearray(read_data))
            else:
                print("Bytes read:%s " % binascii.hexlify(bytearray(read_data)).decode("utf-8"))

    def close(self):
        self.link.close()


def jlink_erase(device, serialno, address, size):
    '''Erase flash.

    This function erase the flash on a device.

    :param device: Name of the device type to be connected. See the list of devices
        in DEVICE_DEF. May be empty. Then function will allow to select one from
        a list. If only one available, immediately use the one.
        If used in conjunction with 'serialno', it is used to verify, whether device
        selected by 'serialno' match expected device type.
    :type device: str

    :param serialno: Serial number of the JLink interface. May be empty. Then function
        will allow to select one from a list. If only one available, immediately use the one.
        If used in conjunction with 'device' parameter, then 'device' parameter is used
        to verify, whether device selected by 'serialno' match expected device type.
    :type serialno: str

    :param address: Starting address in the flash. Currently must be 0.
    :type address: int

    :param size: Size of the area to erase. Currently must be -1.
    :type size: int

    :return: Status of the operation.
    :rtype: Boolean
    '''
    try:
        jlinkexe = ezFlashCLI(device, serialno)
    except Exception as err:
        logging.error(err)
        return False

    try:
        jlinkexe.erase_chip()
    except Exception as err:
        logging.error(err)
        return False
    finally:
        jlinkexe.close()

    return True


def jlink_read_to_file(device, serialno, binfile, address, size):
    '''Read data from a flash and store them in a file.

    This function read data from a flash on the device and write them to a file.

    :param device: Name of the device type to be connected. See the list of devices
        in DEVICE_DEF. May be empty. Then function will allow to select one from
        a list. If only one available, immediately use the one.
        If used in conjunction with 'serialno', it is used to verify, whether device
        selected by 'serialno' match expected device type.
    :type device: str

    :param serialno: Serial number of the JLink interface. May be empty. Then function
        will allow to select one from a list. If only one available, immediately use the one.
        If used in conjunction with 'device' parameter, then 'device' parameter is used
        to verify, whether device selected by 'serialno' match expected device type.
    :type serialno: str

    :param binfile: Output file to stored data read from flash. It is an opened file (by argparse).
    :type binfile: IOBase

    :param address: Starting source address in the flash.
    :type address: int

    :param size: Size of the data to be stored.
    :type size: int

    :return: Status of the operation.
    :rtype: Boolean
    '''
    jlinkexe = None
    try:
        jlinkexe = ezFlashCLI(device, serialno)
        jlinkexe.savefile(binfile, address, size)
    except Exception as err:
        logging.error(err)
        return False
    finally:
        if jlinkexe is not None:
            jlinkexe.close()
        if binfile:
            binfile.close()
 
    return True


def jlink_write_from_file(device, serialno, binfile, address, size):
    '''Write data from a file to flash.

    This function read data from opened file and write them to flash on the device.
    JLinkExe copy whole file to SPI. It may happen, that user want to flash only
    a certain number of bytes. Because of this function make a copy of the binfile
    of a certain size and flash the copy.

    :param device: Name of the device type to be connected. See the list of devices
        in DEVICE_DEF. May be empty. Then function will allow to select one from
        a list. If only one available, immediately use the one.
        If used in conjunction with 'serialno', it is used to verify, whether device
        selected by 'serialno' match expected device type.
    :type device: str

    :param serialno: Serial number of the JLink interface. May be empty. Then function
        will allow to select one from a list. If only one available, immediately use the one.
        If used in conjunction with 'device' parameter, then 'device' parameter is used
        to verify, whether device selected by 'serialno' match expected device type.
    :type serialno: str

    :param binfile: Binary to be stored in the flash. It is an opened file (by argparse).
    :type binfile: IOBase

    :param address: Starting destination address in the flash.
    :type address: int

    :param size: Size of the data to be stored.
    :type size: int

    :return: Status of the operation.
    :rtype: Boolean
    '''
    tmpFile = tempfile.NamedTemporaryFile(suffix='.bin', delete=False)
    tmpFile.write(binfile)
    tmpFile.close()

    jlinkexe = None
    try:
        jlinkexe = ezFlashCLI(device, serialno)
        jlinkexe.loadfile(tmpFile.name, address)
    except ValueError as err:
        logging.error(err)
        return False
    finally:
        if jlinkexe is not None:
            jlinkexe.close()
        os.unlink(tmpFile.name)

    return True

