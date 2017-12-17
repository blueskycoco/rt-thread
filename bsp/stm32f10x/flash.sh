#! /bin/bash -e
scons.bat
jflasharm -openprj$1 -openrtthread.bin,0x08000000 -auto -startapp -exit
