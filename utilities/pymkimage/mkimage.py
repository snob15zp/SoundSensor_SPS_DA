#########################################################################################
# Copyright (C) 2019 Dialog Semiconductor.
# This computer program includes Confidential, Proprietary Information
# of Dialog Semiconductor. All Rights Reserved.
#########################################################################################

import os
import argparse
import struct
import mmap
import time
import binascii
import sys

from math import *
from datetime import datetime
from argparse import ArgumentParser, ArgumentTypeError
from appheader import AppHeader
from bloaderheader import BLoaderHeader
from cfgheader import CfgHeader
from prodheader import ProdHeader

# Here are limits of the size for application and config images.
# Limits are not really strict. Feel free to increase them.
# By observation, the target platrofms are small devices with limited size of flashes or ram.
# Thus if input files are large then certainly something is wrong. The intention of the limits
# is to prevent from generating unnecesarly large files.
MAX_BLOAD_PART = 64*1024
MAX_APP_PART = 64*1024
MAX_CFG_PART = 4*1024
MAX_MULTI_IMG = 1*1024*1024

stripString = ' \t\r\n"'

def check_file_size(f, max_size):
    cur_size = os.path.getsize(f.name)
    if (cur_size > max_size):
        print("ERROR: file", f.name, "has size of", cur_size, "bytes and is larger than", max_size)
        exit(1)

def build_single_image(args):
    if (args.verbose):
        print('Generate application image as:', args.out_file.name)
    # Small check: is input file excessively large?
    check_file_size(args.in_file, MAX_APP_PART)

    # read RAW application binary
    buf = args.in_file.read()

    # prepare application header
    appH = AppHeader()
    version_checker = 0
    for line in args.version_file:
        tmpList = line.split(maxsplit=2)
        if (len(tmpList) == 3 and tmpList[0] =='#define'):
            if (tmpList[1] == 'SW_VERSION'):
                appH.setVersion(tmpList[2].strip(stripString).encode('utf-8'))
                version_checker |= 1
            elif (tmpList[1] == 'SW_VERSION_DATE'):
                tmpDate = datetime.strptime(tmpList[2].strip(stripString), '%Y-%m-%d %H:%M')
                appH.setTimestamp(int(tmpDate.timestamp()))
                version_checker |= 2
    if (version_checker != 3):
        if ((version_checker & 1) == 0):
            print('ERROR: File %s does not contain SW_VERSION' % args.version_file.name)
        if ((version_checker & 2) == 0):
            print('ERROR: File %s does not contain SW_VERSION_DATE' % args.version_file.name)
        exit(1)

    appH.setCRC(binascii.crc32(buf) & 0xffffffff)
    appH.setCodeSize(len(buf))

    # copy header and RAW application binary into application image
    args.out_file.write(appH.header())
    args.out_file.write(buf)
    args.in_file.close()


def build_config_image(args):
    if (args.verbose):
        print('Generate config image as:', args.out_file.name)
    # Small check: is input file excessively large?
    check_file_size(args.in_file, MAX_CFG_PART)

    # read RAW config binary
    buf = args.in_file.read()

    # prepare config header
    cfgH = CfgHeader()
    version_checker = 0
    for line in args.version_file:
        tmpList = line.split(maxsplit=2)
        if (len(tmpList) == 3 and tmpList[0] == '#define'):
            if (tmpList[1] == 'CFG_VERSION'):
                cfgH.setVersion(tmpList[2].strip(stripString).encode('utf-8'))
                version_checker |= 1
            elif (tmpList[1] == 'CFG_ELEMENTS_NUMBER'):
                cfgH.setNItems(int(tmpList[2]))
                if (cfgH.NItems() > 0):
                    version_checker |= 2
    if (version_checker != 3):
        if ((version_checker & 1) == 0):
            print('ERROR: File %s does not contain CFG_VERSION' % args.version_file.name)
        if ((version_checker & 2) == 0):
            print('ERROR: File %s does not contain CFG_ELEMENTS_NUMBER or is invalid' % args.version_file.name)
        exit(1)

    cfgH.setCRC(binascii.crc32(buf) & 0xffffffff)
    cfgH.setDataSize(len(buf))

    # copy header and RAW config binary into config image
    args.out_file.write(cfgH.header())
    args.out_file.write(buf)
    args.in_file.close()


def build_multi_image(args):
    if (args.verbose):
        print('Generate multipart image as:', args.out_file.name)

    # Small check: are input files excessively large?
    check_file_size(args.in_img, MAX_APP_PART)
    check_file_size(args.in_cfg, MAX_CFG_PART)
    if (args.bloader):
        check_file_size(args.bloader, MAX_BLOAD_PART)

    # prepare product header
    prodH = ProdHeader()
    prodH.setImgOffset1(args.off1)
    prodH.setImgOffset2(args.off2)
    prodH.setCfgOffset1(args.off3)
    prodH.setCfgOffset2(args.off4)
    bufHead = prodH.header()

    # prepare application image
    bufImg = args.in_img.read()
    appH = AppHeader()
    appH.check(bufImg)

    # prepare configuration image
    bufCfg = args.in_cfg.read()
    cfgH = CfgHeader()
    cfgH.check(bufCfg)

    if (args.bloader):
        # prepare bloader
        bufBloader = args.bloader.read()
        bloaderH = BLoaderHeader()
        bloaderH.setCodeSize(len(bufBloader))
        bufBloaderHead = bloaderH.header()

    # Calculate size of the final multi-image binary
    partList = [(args.off1, len(bufImg) - 1, 'img1'),
                (args.off2, len(bufImg) - 1, 'img2'),
                (args.off3, len(bufCfg) - 1, 'cfg1'),
                (args.off4, len(bufCfg) - 1, 'cfg2'),
                (args.off5, len(bufHead) - 1, 'header')]

    if (args.bloader):
        partList += [(0, len(bufBloaderHead) + len(bufBloader) - 1, 'bloader')]

    partList.sort()
    indexLast = len(partList)-1;
    outLen = partList[indexLast][0] + partList[indexLast][1] + 1
    if (outLen > MAX_MULTI_IMG):
        print('ERROR: Something must be wrong. The output image cannot have %3.0fMB.' % (outLen/1024/1024))
        exit(1)

    #############################################
    # Sanity check: parts may not overlaps!
    for i in range(len(partList)-1):
        if (partList[i][0] + partList[i][1] >= partList[i+1][0]):
            print('ERROR: some of the buffers overlaps. Please check offsets and sizes of images.')
            for j in range(len(partList)):
                if (j == i) or (j == i+1):
                    overlaps = 'overlaps'
                else:
                    overlaps = ' '
                print('%-10s [%5X, %5X] %s' % (partList[j][2], partList[j][0], partList[j][0] + partList[j][1], overlaps))
            exit(1)
    # End of Sanity check
    ###########################################

    # Prepare the file and mmap
    args.out_file.truncate(outLen)
    outMap = mmap.mmap(args.out_file.fileno(), 0)

    # Copy all parts into multi-image binary
    if (args.bloader):
        outMap[0 : len(bufBloaderHead)] = bufBloaderHead
        outMap[len(bufBloaderHead) : (len(bufBloaderHead) + len(bufBloader))] = bufBloader

    outMap[args.off1 : args.off1 + len(bufImg)]     = bufImg
    outMap[args.off2 : args.off2 + len(bufImg)]     = bufImg

    outMap[args.off3 : args.off3 + len(bufCfg)]     = bufCfg
    outMap[args.off4 : args.off4 + len(bufCfg)]     = bufCfg

    outMap[args.off5 : args.off5 + len(bufHead)]    = bufHead

    # Flush data and close files (except out_file which is closed in "main")
    args.out_file.flush()
    outMap.close()
    if (args.bloader):
        args.bloader.close()
    args.in_img.close()
    args.in_cfg.close()
    if (args.verbose):
        print('Structure of the output image:')
        for i in range(len(partList)):
            print('%-10s [%5X, %5X]' % (partList[i][2], partList[i][0], partList[i][0] + partList[i][1]))



# helper method for argparse that allow enter integer values in both decimal and hexadecimal format
def any_int(x):
    return eval(x)


###################
#       MAIN      #
###################

parser = argparse.ArgumentParser(description='Tool to prepare image files used for DA14531.')
parser.add_argument('--verbose', '-v', help='verbose', action='count')

# mkimage SINGLE
# Syntax: mkimage.exe single in_file version_file out_file
subparsers = parser.add_subparsers(help='Type of image to generate')

parser_single = subparsers.add_parser('single', help='SUOTA compatible')
parser_single.add_argument('in_file', type=argparse.FileType('rb'), help='RAW binary to be inserted into image')
parser_single.add_argument('version_file', type=argparse.FileType('rt'), help='C header with *SW_VERSION* definitions')
parser_single.add_argument('out_file', type=argparse.FileType('wb'), help='output image in SUOTA format')
parser_single.set_defaults(func=build_single_image)

# mkimage CFG
# Syntax: mkimage.exe cfg in_file version_file out_file
parser_cfg = subparsers.add_parser('cfg', help='configuration')
parser_cfg.add_argument('in_file', type=argparse.FileType('rb'), help='configuration binary to be inserted into image')
parser_cfg.add_argument('version_file', type=argparse.FileType('rt'), help='C header with *CFG_VERSION* definitions')
parser_cfg.add_argument('out_file', type=argparse.FileType('wb'), help='output image')
parser_cfg.set_defaults(func=build_config_image)

# mkimage MULTI
# Syntax: mkimage.exe multi in_img1 off1 off2 cfg_struct_cfg off3 off4 off5 out_file [bloader]
parser_multi = subparsers.add_parser('multi', help='multi-part', description='Pack together application image (output from "single") and configuration image (output from "cfg") in to one image')
parser_multi.add_argument('in_img', type=argparse.FileType('rb'), help='file with application image to be inserted into output image')
parser_multi.add_argument('off1', type=any_int, help='offset for application image')
parser_multi.add_argument('off2', type=any_int, help='offset for alternative application image (filled with the same in_img)')
parser_multi.add_argument('in_cfg', type=argparse.FileType('rb'), help='file with configuration image to be inserted into output image')
parser_multi.add_argument('off3', type=any_int, help='offset for configuration image')
parser_multi.add_argument('off4', type=any_int, help='offset for alternative configuration image (filled with the same in_cfg)')
parser_multi.add_argument('off5', type=any_int, help='offset for product header')
parser_multi.add_argument('out_file', type=argparse.FileType('wb+'), help='output image')
parser_multi.add_argument('bloader', type=argparse.FileType('rb'), nargs='?', help='file with boot loader to be stored at offset 0')
parser_multi.set_defaults(func=build_multi_image)

args = parser.parse_args()

try:
    args.func(args)
except AttributeError:
    parser.print_usage()
    sys.exit(1)
args.out_file.close()
print('SUCCESS')
