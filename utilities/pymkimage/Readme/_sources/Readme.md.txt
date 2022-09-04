pymkimage
=========

mkimage is a CLI python application for
1. building SUOTA ready s/w images,
2. adding header in raw binary configuration files,
3. creating multi-image for first time programming


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
1. Open any command prompt,
2. navigate to the folder that contains configurator
3. build the executable 

```pyinstaller37.exe --onefile mkimage.py --nowindowed --distpath . ```
