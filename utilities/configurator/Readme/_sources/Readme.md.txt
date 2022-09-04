DA14531 Configurator
====================

Configurator is a graphic user interface application in python to create/read/modify configuration structure files in binary format.
The user should be able to:

* Open an existing configuration file and read the included elements and their values.
* Modify the configuration file by adding/removing elements or change the value of included elements. Save changes in the same file or in a new file 
* Create a new configuration file, modify it, as described previous boolet, and save.

The list of Groups, element IDs and types will be inserted to the application through an XML file, which describs all specified groups, element IDs and their corresponding types and maximum sizes. 
The application will open and read the XML file located in a predefined location on startup.


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
1. ```pip3 install PyInstaller==3.6```
2. rename pyinstaller.exe to pyinstaller37.exe

Create the executable under current folder
------------------------------------------
1. Open any command prompt,
2. navigate to the folder that contains configurator,
3. build the executable 

 ```pyinstaller37.exe -n configurator --onefile da14531_configurator.py --windowed -i Dialog_D-Symbol_Color_RGB_Small_icon.ico --add-data "Dialog_D-Symbol_Color_RGB_Small_icon.ico;." --distpath . ```