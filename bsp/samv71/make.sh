#! /bin/bash -e

scons.bat
openocd -f openocd.cfg -c "flash_image" -s C:/cygwin/usr/local/share/openocd/scripts
