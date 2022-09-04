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

from pyjlink import *

import json
import struct

SPI_FLASH_PAGE_SIZE         = 256
SPI_FLASH_SECTOR_SIZE       = 4096


class HW_QSPI_COMMON_CMD(IntEnum):
    WRITE_STATUS_REGISTER =      0x01,
    PAGE_PROGRAM          =      0x02,
    READ_DATA             =      0x03,
    WRITE_DISABLE         =      0x04,
    READ_STATUS_REGISTER  =      0x05,
    WRITE_ENABLE          =      0x06,
    SECTOR_ERASE          =      0x20,
    QUAD_PAGE_PROGRAM     =      0x32,
    QPI_PAGE_PROGRAM      =      0x02,
    BLOCK_ERASE           =      0x52,
    CHIP_ERASE            =      0xC7,
    FAST_READ_QUAD        =      0xEB,
    READ_JEDEC_ID         =      0x9F,
    EXIT_CONTINUOUS_MODE  =      0xFF,
    RELEASE_POWER_DOWN    =      0xAB,
    ENTER_POWER_DOWN      =      0xB9,
    FAST_READ_QUAD_4B     =      0xEC,
    SECTOR_ERASE_4B       =      0x21,
    QUAD_PAGE_PROGRAM_4B  =      0x34,
    ENTER_QPI_MODE        =      0x38,    # Requires single mode for the command entry!
    EXIT_QPI_MODE         =      0xFF     # Requires quad mode for the command entry!


class DA14xxx:
    """ Handler for the Smartbond devices Flash memory
    """

    def __init__(self, link):

        self.flash_address_size = 3
        self.polling_interval = 0.01
        self.wait_timeout = 30

        self.link = link


class DA1453x_DA1458x(DA14xxx):
    pass


class DA14531(DA1453x_DA1458x):
    CLK_AMBA_REG        = 0x50000000
    SET_FREEZE_REG      = 0x50003300
    PAD_LATCH_REG       = 0x5000030C
    SYS_CTRL_REG        = 0x50000012
    HWR_CTRL_REG        = 0x50000300
    CLK_PER_REG         = 0x50000004

    SPI_CTRL_REG        = 0x50001200
    SPI_CONFIG_REG      = 0x50001204
    SPI_CLOCK_REG       = 0x50001208
    SPI_FIFO_CONFIG_REG = 0x5000120C
    SPI_FIFO_STATUS_REG = 0x50001218
    SPI_FIFO_READ_REG   = 0x5000121C
    SPI_FIFO_WRITE_REG  = 0x50001220
    SPI_CS_CONFIG_REG   = 0x50001224

    P0_DATA_REG         = 0x50003000
    P00_MODE_REG        = 0x50003006
    P00_MODE_REG_RESET  = 0x00000200
    
    SPI_PORT            = 0
    SPI_CLK_PIN         = 4
    SPI_CS_PIN          = 1
    SPI_DI_PIN          = 3
    SPI_DO_PIN          = 0

    OUTPUT              = 0x300
    INPUT               = 0
    # Word Size 8 bits
    SPI_MODE_8BIT       = 0
    # Word Size 16 bits
    SPI_MODE_16BIT      = 1
    # Word Size 32 bits
    SPI_MODE_32BIT      = 2

    def __init__(self, link):
        DA1453x_DA1458x.__init__(self, link)

    def GPIO_SetPinFunction(self, port, pin, mode, function):
        data_reg = self.P0_DATA_REG + (port << 5)
        mode_reg = data_reg + 0x6 + (pin << 1)
        self.link.wr_mem(16, mode_reg, mode|function)

    def GPIO_SetActive(self, port, pin):
        data_reg = self.P0_DATA_REG + (port << 5)
        set_data_reg = data_reg + 2
        self.link.wr_mem(16, set_data_reg, 1 << pin)

    def spi_set_bitmode(self, spi_wsz):
        if spi_wsz == self.SPI_MODE_16BIT:
            self.SetBits16(self.SPI_CONFIG_REG, 0x7c, 15)
        elif spi_wsz == self.SPI_MODE_32BIT:
            self.SetBits16(self.SPI_CONFIG_REG, 0x7c, 31)
        else:
            self.SetBits16(self.SPI_CONFIG_REG, 0x7c, 7)

    def shift16(self, a):
        shift = 0
        while not (a & 0x1):
           shift += 1
           a = (a >> 1)
        return shift

    def SetWord16(self, addr, data):
        self.link.wr_mem(16, addr, data)

    def SetBits16(self, addr, bitfield_mask, data):
        reg = self.link.rd_mem(16, addr, 1)[0]
        reg = (reg & (~ bitfield_mask)) & 0xFFFF
        wr_data = reg | (data << (self.shift16(bitfield_mask)))
        self.link.wr_mem(16, addr, wr_data)

    def spi_cs_low(self):
        self.SetBits16(self.SPI_CTRL_REG, 0x20, 0)  # release reset fifo
        self.SetWord16(self.SPI_CS_CONFIG_REG, 1)

    def spi_cs_high(self):
        self.SetWord16(self.SPI_CS_CONFIG_REG, 0)
        self.SetBits16(self.SPI_CTRL_REG, 0x20, 1)  # reset fifo

    def spi_access8(self, dataToSend):
        dataRead = 0

        # Clear Tx, Rx and DMA enable paths in Control Register
        self.SetBits16(self.SPI_CTRL_REG, 0x1F, 0)

        # Enable TX path
        self.SetBits16(self.SPI_CTRL_REG, 0x2, 1)
        # Enable RX path
        self.SetBits16(self.SPI_CTRL_REG, 0x4, 1)
        # Enable SPI
        self.SetBits16(self.SPI_CTRL_REG, 0x1, 1)

        # Write (low part of) dataToSend
        self.SetWord16(self.SPI_FIFO_WRITE_REG, dataToSend)

        # Wait while RX FIFO is empty
        while (self.link.rd_mem(16, self.SPI_FIFO_STATUS_REG, 1)[0] & 0x1000) != 0:
            pass

        dataRead = self.link.rd_mem(16, self.SPI_FIFO_READ_REG, 1)[0] & 0xFF

        # Wait until transaction is finished and SPI is not busy
        while (self.link.rd_mem(16, self.SPI_FIFO_STATUS_REG, 1)[0] & 0x8000) != 0:
            pass
        return dataRead

    def flash_init(self):
        """ Initiallize flash controller and make sure the
            Flash device exits low power mode

            Args:
                None
        """
        self.SetWord16(self.CLK_AMBA_REG, 0x00)             # set clocks (hclk and pclk ) 16MHz
        self.SetWord16(self.SET_FREEZE_REG, 0x8)            # stop watch dog
        self.SetBits16(self.PAD_LATCH_REG, 0x1, 1)          # open pads
        self.SetBits16(self.SYS_CTRL_REG, 0x0180, 0x3)      # SWD_DIO = P0_10
        self.SetWord16(self.HWR_CTRL_REG, 1)                # disable HW reset

        self.GPIO_SetPinFunction(self.SPI_PORT, self.SPI_CS_PIN, 0x300, 29)     # SPI_CS
        self.GPIO_SetActive(self.SPI_PORT, self.SPI_CS_PIN)
        self.GPIO_SetPinFunction(self.SPI_PORT, self.SPI_CLK_PIN, 0x300, 28)    # SPI_CLK
        self.GPIO_SetPinFunction(self.SPI_PORT, self.SPI_DO_PIN, 0x300, 27)     # SPI_D0
        self.GPIO_SetPinFunction(self.SPI_PORT, self.SPI_DI_PIN, 0, 26)         # SPI_DI

        self.SetBits16(self.CLK_PER_REG, 0x400, 1)
        # Disable SPI / Reset FIFO in SPI Control Register
        self.SetWord16(self.SPI_CTRL_REG, 0x0020)   # fifo reset
        # Set SPI Word length
        self.spi_set_bitmode(self.SPI_MODE_8BIT)
        # Set SPI Mode (CPOL, CPHA)
        self.SetBits16(self.SPI_CONFIG_REG, 0x0003, 0)  # mode 0
        # Set SPI Master/Slave mode
        self.SetBits16(self.SPI_CONFIG_REG, 0x80, 0)    # master mode

        # Set SPI FIFO threshold levels to 0
        self.SetWord16(self.SPI_FIFO_CONFIG_REG, 0)
        # Set SPI clock in async mode (mandatory)
        self.SetBits16(self.SPI_CLOCK_REG, 0x0080, 1)

        # Set SPI master clock speed
        self.SetBits16(self.SPI_CLOCK_REG, 0x007F, 7)    # 2MHz
        # Set SPI clock edge capture data
        self.SetBits16(self.SPI_CTRL_REG, 0x0040, 0)

    def release_reset(self):
        """
            On 531 the reset pin is shared with the default flash MOSI pin.
            The function restore the pin into reset mode

        """
        self.link.wr_mem(16, self.HWR_CTRL_REG, 0x0)
        self.link.wr_mem(16, self.P00_MODE_REG, self.P00_MODE_REG_RESET)

    def read_flash(self, address, length):
        read_data = []

        self.flash_init()

        self.spi_cs_low()
        self.spi_access8(HW_QSPI_COMMON_CMD.READ_DATA)
        self.spi_access8(address & 0xFF)
        self.spi_access8((address >> 8) & 0xFF)
        self.spi_access8((address >> 16) & 0xFF)
        while length:
            read_data.append(self.spi_access8(0xFF))
            length -= 1

        self.spi_cs_high()
        self.release_reset()
        return (read_data)

