from struct import *
import zlib
import binascii
import re

"""
This module contains Config class
"""

SIGNATURE = 0x5470
VALID_FLAG = 0xAA
STRUCTURE_ID = 0x00
ENCRYPTED = 0x01
NO_ENCRYPTION = 0x00
HEADER_LENGTH = 64


class Config:
    def __init__(self):
        self.config_header = dict()
        self.config_header["signature"] = SIGNATURE
        self.config_header["valid_flag"] = VALID_FLAG
        self.config_header["structure_id"] = STRUCTURE_ID
        self.config_header["crc"] = 0
        self.config_header["version"] = bytes(16)
        self.config_header["data_size"] = 0
        self.config_header["number_of_items"] = 0
        self.config_header["encryption_flag"] = NO_ENCRYPTION
        self.config_header["reserved"] = bytes(36)

        self.cfg_binary = b''
        self.elements = list()

    def get_elements(self):
        return self.elements

    def set_version(self, version):
        self.config_header["version"] = version

    def get_version(self):
        return self.config_header["version"]

    def create_header_bin(self):
        version_bytes = str.encode(self.config_header["version"])
        header_binary = pack('<hBBL16shBB36s', self.config_header["signature"],
                                               self.config_header["valid_flag"],
                                               self.config_header["structure_id"],
                                               self.config_header["crc"],
                                               version_bytes,
                                               self.config_header["data_size"],
                                               self.config_header["number_of_items"],
                                               self.config_header["encryption_flag"],
                                               self.config_header["reserved"])
        return header_binary

    def get_binary(self, elements):
        self.element_count = len(elements)
        self.cfg_binary = b''

        self.config_header["number_of_items"] = len(elements)

        for elem in elements:
            fmt = '<hB%ds' % int(self.get_element_length(elem))
            if elem['type'] == 'string':
                val_bytes = bytes(elem["value"], 'utf-8')
            elif elem['type'] == 'bd_address_t':
                #### Change the byte order using [::-1] ####
                val_bytes = bytes.fromhex(elem['value'])[::-1]
            elif elem['type'] == 'array':
                val_bytes = bytes.fromhex(elem['value'])
            elif elem['type'] == 'gpio_pin_t':
                # # Get values between brackets, str is Port: [09], Pin: [63]
                result = re.findall(r'\[(.*?)\]', elem['value'])
                port = hex(int(result[0]))[2:]
                pin = hex(int(result[1]))[2:]
                val = int(port.zfill(2) + pin.zfill(2), 16)
                val_bytes = val.to_bytes(self.get_element_length(elem), byteorder='little')
            else:
                val = int(elem['value'])
                val_bytes = val.to_bytes(self.get_element_length(elem), byteorder='little')
#                val_bytes = bytes(elem['value'], 'utf8')

            self.cfg_binary = self.cfg_binary + pack(fmt, int(elem["id"], 0), self.get_element_length(elem), val_bytes)
        self.config_header["data_size"] = len(self.cfg_binary)
        self.config_header["crc"] = zlib.crc32(self.cfg_binary) & 0xffffffff

        self.cfg_binary = self.create_header_bin() + self.cfg_binary  # add config structure header

        return self.cfg_binary

    def get_element_length(self, elem):
        def el_size(type):
            if type == "string":
                return len(elem["value"])
            if type == "array" or type == "bd_address_t":
                return len(elem["value"]) // 2
            else:
                return elem["size"]

        elem_sizes = {
            "uint8_t": 1,
            "int8_t": 1,
            "uint16_t": 2,
            "int16_t": 2,
            "uint32_t": 4,
            "int32_t": 4,
            "gpio_pin_t": 2,
        }
        return elem_sizes.get(elem["type"], el_size(elem["type"]))

    def set_binary(self, binary):
        # parse header binary to confg_header dict
        try:
            header = unpack_from('<hBBL16shBB36s', binary)
            self.config_header["signature"] = header[0]
            self.config_header["valid_flag"] = header[1]
            self.config_header["structure_id"] = header[2]
            self.config_header["crc"] = header[3]
            self.config_header["version"] = str(header[4], "utf-8").rstrip('\0')
            self.config_header["data_size"] = header[5]
            self.config_header["number_of_items"] = header[6]
            self.config_header["encryption_flag"] = header[7]
            self.config_header["reserved"] = header[8]
        except error:
            return False

        size = len(binary)
        offset = HEADER_LENGTH
        elements = list()
        while offset < size:
            el = unpack_from('<hB', binary, offset)
            element_id = el[0]
            element_size = el[1]
            offset += 3
            value = unpack_from('<%ds' % element_size, binary, offset)
            element_value = value[0]
            offset += element_size
            elements.append({"id": element_id, "value": element_value, "size": element_size})

        self.elements = elements
        return True
