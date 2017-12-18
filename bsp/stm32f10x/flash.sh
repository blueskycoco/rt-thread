#! /bin/bash -e
scons.bat
jflash -openprj$1 -openrtthread.bin,0x08000000 -auto -startapp -exit
