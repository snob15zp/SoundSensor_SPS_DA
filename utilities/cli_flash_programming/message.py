"""Message communication layer
Message Manager is an object that prepare messages, send them and receive responds.
Responds may consist only from status, but may include requested data or additional error code.

This object shall allow for program level for simple issuing commands without the need
of knowing any of protocol details.
"""
import binascii
import logging
from link import LinkManager
from struct import pack, unpack, calcsize
from enum import IntEnum


class RequestOpCode(IntEnum):
    ACTION_NO_ACTION        = 0x00
    ACTION_READ_VERSION     = 0x10
    ACTION_UART_BAUD        = 0x30
    ACTION_UART_GPIOS       = 0x31
    ACTION_GPIOS            = 0x32
    ACTION_OTP_READ         = 0x80
    ACTION_OTP_WRITE        = 0x81
    ACTION_SPI_READ         = 0x90
    ACTION_SPI_WRITE        = 0x91
    ACTION_SPI_ERASE        = 0x92
    ACTION_SPI_ID           = 0x93
    ACTION_SPI_ERASE_BLOCK  = 0x94
    ACTION_SPI_GPIOS        = 0x95
    ACTION_SPI_IS_EMPTY     = 0x96
    ACTION_SPI_INIT         = 0x97
    ACTION_EEPROM_READ      = 0xA0
    ACTION_EEPROM_WRITE     = 0xA1
    ACTION_EEPROM_ERASE     = 0xA2
    ACTION_I2C_GPIOS        = 0xA3
    ACTION_EEPROM_INIT      = 0xA4
    ACTION_GPIO_WD          = 0xB0
    ACTION_PLATFORM_RESET   = 0xC0


uartReqHead = ('>'     # Big endian
               'H'     # 2 - Total length
               'I'     # 4 - CRC
               )


class ResponseStatus(IntEnum):
    ACTION_READY            = 0x5A
    ACTION_CONTENTS         = 0x82
    ACTION_OK               = 0x83
    ACTION_ERROR            = 0x84
    ACTION_DATA             = 0x85      # unused, but reserved
    ACTION_INVALID_COMMAND  = 0x86
    ACTION_INVALID_CRC      = 0x87


uartRespHead = ('>'     # Big endian
                'H'     # 2 - Total length
                'I'     # 4 - CRC
                )


class MessageManager():
    def __init__(self, port=None, device=None, selector=None, userUartFirmware=None):
        """Constructor: open serial port and auto connect to the target

        In most cases, user will relay on auto detection of the serial port, but
        explicit selection of serial port is supported (mainly to support
        custom boards without JLink on the board).

        The MessageManager automatically initiate the connection.

        :param port: Used for force usage of a specific serial port.
        :type port: str

        :param device: Used for force usage of a specific device type (DA14531 or DA14585).
        :type device: str

        :param userUartFirmware: The file descriptor of the executable binary file for alternative UART firmware
            known also as flash_programmer. If omitted, try to use one from well known location.
            (default is None)
        :type userUartFirmware: class '_io.BufferedReader'
        """
        self.device = device
        self.lm = LinkManager(port=port, device=self.device, selector=selector, userUartFirmware=userUartFirmware)
        self.connect()

    def connect(self):
        """Connect to the target

        This function is required after issuing an RESET command to reinit connection.
        """
        self.lm.connect()

    def build_msg(self, OpCode, data=b''):
        """Build header of a message

        This function only prepare the header of a message, but do not send it.
        The header consist from total length, CRC and opCode.

        :param OpCode: Command code.
        :type OpCode: int/enum

        :param data: Data to be send after message. The data are not include in the returned
            header of a message, but is required here to calculate CRC32.
        :type data: bytes

        :return: The header of a message ready for sending
        :rtype: bytes
        """
        bufMsg = pack('B', OpCode)
        crc32 = binascii.crc32(bufMsg+data) & 0xffffffff
        bufHeadSect = pack(uartReqHead, len(bufMsg) + len(data), crc32)

        return bufHeadSect + bufMsg + data

    def send(self, data):
        """Simple sending of data

        This method send any data without knowing the content. It may be a header.

        :param data: Data to be send
        :type data: bytes
        """
        if len(data):
            if len(data) < 128:
                logging.debug('send: %s', binascii.hexlify(data))
            self.lm.write(data)

    def recv(self, executionTime=0):
        """Simple recieving of datg

        This method wait and read data, but unlike send() it make simple analyze the content.
        It wait first for the common respond header, extract the payload size and CRC32 then
        read for following data (depend on payload size).
        After data reciev, validate the CRC32.

        :param executionTime: Some commands may take execution time eg SPI_READ. In such cases
            a calling function need to inform, how long to wait for the execution of the command.
        :type executionTime: float

        :return: Calling function get status and data. Note, data may include extended error code.
        :rtype: tuple(int, bytes)
        """
        data = self.lm.read(calcsize(uartRespHead), executionTime=executionTime)
        if not data:
            raise OSError("UART read error: no data received")
        logging.debug("recv: %s", binascii.hexlify(data))
        payload_len, crc32 = unpack(uartRespHead, data)
        if payload_len < 1:     # Please keep in mind, "status" exists in each response
            logging.info('\nunexpected length of data ready to read from UART: %d' % payload_len)
            exit(1)
        data = self.lm.read(payload_len)
        if not data or payload_len != len(data):
            logging.info('\nexpected %d bytes of data from target device, but received %d bytes'
                         % (payload_len, len(data)))
        elif crc32 != (binascii.crc32(data) & 0xffffffff):
            logging.info('\nCRC32 mismatch, header crc32: 0x%08X, calculated from data: 0x%08X'
                         % (crc32, binascii.crc32(data) & 0xffffffff))
            logging.info('received data:', binascii.hexlify(data))
        logging.debug(' %s', binascii.hexlify(data))
        status = data[0]
        if (((status == ResponseStatus.ACTION_CONTENTS) and (payload_len <= 1)) or
                ((status == ResponseStatus.ACTION_OK) and (payload_len > 1))):
            logging.info('respond status: 0x%02x and payload_len: %d mismatch' % (status, payload_len))

        return status, data[1:]

    def getExecutionTime(self, OpCode):
        """Get maximal execution time of a command

        After issuing a command to the target, the respond may come with a delay which do not
        depend only on the serial port speed. This method return a maximal execution time
        of commands in seconds. If execution of a command take longer than this time, it will be
        considered as an error.

        :param OpCode: Commands ID
        :type OpCode: int/enum

        :return: time of execution in seconds
        :rtype: float
        """
        executionTimes = {
            str(RequestOpCode.ACTION_SPI_ERASE): 10,
            str(RequestOpCode.ACTION_SPI_WRITE): 0.6,
            str(RequestOpCode.ACTION_SPI_READ): 2,
            str(RequestOpCode.ACTION_SPI_ERASE_BLOCK): 5
        }
        try:
            return executionTimes[str(OpCode)]
        except KeyError:
            return 0.2

    def send_and_recv(self, OpCode, data=b''):
        """Core of the MessageManager - send commands and receive responds

        This is core method of the MessageManager. It hide as much as possible
        details of the protocol. It build message's header, send the header and data
        then wait for respond and interpret the recieved payload.

        :param OpCode: The command to be issued.
        :type OpCode: int/enum

        :param data: Data to be send.
        :type data: bytes

        :return: Method return interpreded data, thus depend on the command.
            By default that are: status, extended error code and data
            For ACTION_PLATFORM_RESET: no return
            For ACTION_SPI_ID: status, extended error code and manufacture ID, device ID 1 and device ID 2
        :rtype: tuple
        """
        # This read bellow is not required by the protocol. But it turn out, that if an error happen during
        # transmission, host may receive unexpected data, which in fact are remaining respond from previous
        # command. Let read them from the target. But keep in mind, this SHOULD never happen.
        readDummy = self.lm.read(1)
        while readDummy:
            logging.debug('!!!!!!!!!! dummy read success: 0x%02x' % readDummy[0])
            readDummy = self.lm.read(1)

        logging.info('CMD: OpCode = 0x%x', OpCode)
        msg = self.build_msg(OpCode, data)
        self.send(msg)
        if OpCode == RequestOpCode.ACTION_PLATFORM_RESET:
            return
        status, respData = self.recv(self.getExecutionTime(OpCode))

        error = 0
        additional_error_code = None
        if status == ResponseStatus.ACTION_INVALID_CRC or \
                status == ResponseStatus.ACTION_INVALID_COMMAND:
            error = 1
            logging.error('status: invalid [0x%x] ', status)
            return status, error, additional_error_code
        elif status == ResponseStatus.ACTION_ERROR:
            error = 1
            try:
                additional_error_code = unpack('>I', respData[0:4])[0]
                logging.error('status: error: 0x%08x', additional_error_code)
            except:
                pass
            return status, error, additional_error_code
        elif status == ResponseStatus.ACTION_CONTENTS:
            logging.debug('status: success with contents.')
            logging.debug('data: %s', binascii.hexlify(respData))
            return status, error, respData
        elif status == ResponseStatus.ACTION_OK:
            logging.info('status: success')
            return status, error, additional_error_code
        else:
            logging.error('Unknown OpCode: %x' % OpCode)
            error = 1
            return status, error, additional_error_code
