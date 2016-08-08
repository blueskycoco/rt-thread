#! /bin/bash
scons.bat
openocd.exe -s c:/cygwin/usr/local/share/openocd/scripts -f openocd.cfg -c "flash_image"
