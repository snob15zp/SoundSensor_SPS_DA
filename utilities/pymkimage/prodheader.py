from struct import *

ProdHeaderStruct = ( '<'   # Little endian + sizes of fields forced by python
                    'H'   # 2 bytes Signature
                    'H'   # 2 bytes Version
                    'I'   # 4 bytes Image Bank1 Offset
                    'I'   # 4 bytes Image Bank2 Offset
                    'I'   # 4 bytes Config Bank1 Offset
                    'I'   # 4 bytes Config Bank2 Offset
                )

PRODUCT_SIGNATURE = 0x5270

class ProdHeader(Struct):
    def __init__(self):
        super(ProdHeader, self).__init__(ProdHeaderStruct)
        self.signature = PRODUCT_SIGNATURE
        self.version = 0
        self.imgOffset1 = 0x00000
        self.imgOffset2 = 0x0C000
        self.cfgOffset1 = 0x18000
        self.cfgOffset2 = 0x19000

    def Signature(self):
        return self.signature
      
    def Version(self):
        return self.version

    def ImgOffset1(self):
        return self.imgOffset1

    def ImgOffset2(self):
        return self.imgOffset2

    def CfgOffset1(self):
        return self.cfgOffset1

    def CfgOffset2(self):
        return self.cfgOffset2

    def setSignature(self, newSignature):
        self.signature = newSignature
      
    def setVersion(self, newVersion):
        self.version = newVersion

    def setImgOffset1(self, newOffset):
        self.imgOffset1 = newOffset

    def setImgOffset2(self, newOffset):
        self.imgOffset2 = newOffset

    def setCfgOffset1(self, newOffset):
        self.cfgOffset1 = newOffset

    def setCfgOffset2(self, newOffset):
        self.cfgOffset2 = newOffset

    def header(self):
        return self.pack(self.signature,
                    self.version,
                    self.imgOffset1,
                    self.imgOffset2,
                    self.cfgOffset1,
                    self.cfgOffset2,
                   )

