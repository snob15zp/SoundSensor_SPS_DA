from struct import *

BLoaderHeaderStruct = ( '>'   # big endian + sizes of fields forced by python
                    'H'   # 2 bytes Signature
                    'H'   # 2 byte  reserved
                    'I'   # 4 bytes BLoader Code Size
                )

BLOAD_SIGNATURE = 0x7050

class BLoaderHeader(Struct):
    def __init__(self):
        super(BLoaderHeader, self).__init__(BLoaderHeaderStruct)
        self.signature = BLOAD_SIGNATURE
        self.reserved = 0
        self.codesize = -1

    def Signature(self):
        return self.signature

    def CodeSize(self):
        return self.codesize

    def setSignature(self, newSignature):
        self.signature = newSignature

    def setCodeSize(self, newSize):
        self.codesize = newSize

    def header(self):
        return self.pack(self.signature,
                    self.reserved,
                    self.codesize,
                   )

