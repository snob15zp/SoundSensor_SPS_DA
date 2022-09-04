cli_flash_programming
=====================

cli_flash_programming is a CLI python application for

* erasing whole or part of the SPI FLASH,
* reading whole or specific number of bytes from the SPI FLASH and  
* writing a binary or specific number of bytes to the SPI FLASH, 

either using a UART or a JTAG connection.


Requirements
============
Pyhton
------
Required version is Python 3.7.3

Installation
------------
1. Download Python installer from `Python 3.7.3 <https://www.python.org/ftp/python/3.7.3/python-3.7.3-amd64.exe>`_
2. Add Python to PATH


PyInstaller
-----------
PyInstaller freezes (packages) Python applications into stand-alone executables.

Installation
------------
1. ``` pip3 install PyInstaller==3.6 ```
2. rename pyinstaller.exe to pyinstaller37.exe

Create the executable under current folder
------------------------------------------
1. Open any command prompt
2. Navigate to folder that contains cli_flash_programming
3. type ```pyinstaller37.exe --onefile cli_flash_programming.py --nowindowed --add-data "Dialog_D-Symbol_Color_RGB_Small_icon.ico;." --add-data ".\third-party\*;." --distpath .  ```
