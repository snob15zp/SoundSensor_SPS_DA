import binascii
from struct import *

AppHeaderStruct = ( '<'   # Little endian + sizes of fields forced by python
                    'H'   # 2 bytes Signature
                    'B'   # 1 byte  Valid Flag
                    'B'   # 1 byte  Image ID
                    'I'   # 4 bytes Code Size
                    'I'   # 4 bytes CRC
                    '16s' # 16 bytes Version
                    'I'   # 4 bytes Timestamp
                    'B'   # 1 byte  Encryption
                    '31s' # 31 bytes reserved
                )

APP_SIGNATURE = 0x5170

class AppHeader(Struct):
    def __init__(self):
        super(AppHeader, self).__init__(AppHeaderStruct)
        self.signature = APP_SIGNATURE
        self.valid = 0xAA
        self.id = 0x00
        self.codesize = -1
        self.crc = 0xDEADBEEF
        self.version = b'bad version'
        self.timestamp = 0
        self.encryption = 0
        self.reserved =  b''

    def Signature(self):
        return self.signature
      
    def Valid(self):
        return self.valid

    def ImageID(self):
        return self.id

    def CodeSize(self):
        return self.codesize

    def CRC(self):
        return self.crc

    def Version(self):
        return self.version

    def Timestamp(self):
        return self.timestamp

    def Encryption(self):
        return self.encryption

    def setSignature(self, newSignature):
        self.signature = newSignature
      
    def setValid(self, newState):
        self.valid = newState

    def setImageID(self, newID):
        self.id = newID

    def setCodeSize(self, newSize):
        self.codesize = newSize

    def setCRC(self, newCRC):
        self.crc = newCRC

    def setVersion(self, newVersion):
        self.version = newVersion

    def setTimestamp(self, newTimestamp):
        self.timestamp = newTimestamp

    def setEncryption(self, newState):
        self.encryption = newState

    def header(self):
        return self.pack(self.signature,
                    self.valid,
                    self.id,
                    self.codesize,
                    self.crc,
                    self.version,
                    self.timestamp,
                    self.encryption,
                    self.reserved
                   )

    def check(self, newContent):
        self.signature, self.valid, self.id, self.codesize, self.crc, \
        self.version, self.timestamp, self.encryption, self.reserved \
           = self.unpack(newContent[0:self.size])

        if self.signature != APP_SIGNATURE:
            print('Application image error: improper signature 0x%04X' % self.signature)
        elif self.valid != 0xAA:
            print('Application image error: improper valid tag 0x%02X' % self.valid)
        elif self.codesize != len(newContent) - self.size:
            print('Application image error: improper size 0x%08X' % self.codesize)
        elif self.crc != (binascii.crc32(newContent[self.size:]) & 0xffffffff):
            print('Application image error: improper crc 0x%08X' % self.crc)
        else:
            return
        exit(-1)

    def headSize(self):
        return self.size

