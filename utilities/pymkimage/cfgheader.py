import binascii
from struct import *

CfgHeaderStruct = ( '<'   # Little endian + sizes of fields forced by python
                    'H'   # 2 bytes Signature
                    'B'   # 1 byte  Valid Flag
                    'B'   # 1 byte  Structe ID
                    'I'   # 4 bytes CRC
                    '16s' # 16 bytes Version
                    'H'   # 2 bytes Data Size
                    'B'   # 1 byte  Number of items
                    'B'   # 1 byte  Encryption
                    '36s' # 36 bytes reserved
                )
CFG_SIGNATURE = 0x5470

class CfgHeader(Struct):
    def __init__(self):
        super(CfgHeader, self).__init__(CfgHeaderStruct)
        self.signature = CFG_SIGNATURE
        self.valid = 0xFF
        self.id = 0x00
        self.crc = 0xDEADBEEF
        self.version = b'bad version'
        self.datasize = 0
        self.items = 0
        self.encryption = 0
        self.reserved =  b''

    def Signature(self):
        return self.signature
      
    def Valid(self):
        return self.valid

    def StructID(self):
        return self.id

    def DataSize(self):
        return self.datasize

    def CRC(self):
        return self.crc

    def Version(self):
        return self.version

    def NItems(self):
        return self.items

    def Encryption(self):
        return self.encryption

    def setSignature(self, newSignature):
        self.signature = newSignature
      
    def setValid(self, newState):
        self.valid = newState

    def setStructID(self, newID):
        self.id = newID

    def setDataSize(self, newSize):
        self.datasize = newSize

    def setCRC(self, newCRC):
        self.crc = newCRC

    def setVersion(self, newVersion):
        self.version = newVersion

    def setNItems(self, newNItems):
        self.items = newNItems

    def setEncryption(self, newState):
        self.encryption = newState

    def header(self):
        return self.pack(self.signature,
                    self.valid,
                    self.id,
                    self.crc,
                    self.version,
                    self.datasize,
                    self.items,
                    self.encryption,
                    self.reserved
                   )
    def check(self, newContent):
        self.signature, self.valid, self.id, self.crc, self.version, \
        self.datasize, self.items, self.encryption, self.reserved \
           = self.unpack(newContent[0:self.size])

        if self.signature != CFG_SIGNATURE:
            print('Configuration image error: improper signature 0x%04X' % self.signature)
        elif self.valid != 0xAA:
            print('Configuration image error: improper valid tag 0x%02X' % self.valid)
        elif self.datasize != len(newContent) - self.size:
            print('Configuration image error: improper size 0x%08X' % self.datasize)
        elif self.crc != (binascii.crc32(newContent[self.size:]) & 0xffffffff):
            print('Configuration image error: improper crc 0x%08X' % self.crc)
        else:
            return
        exit(-1)

    def headSize(self):
        return self.size

