#! /bin/bash -e
scons.bat
jflash -openprj$1 -openstm32.bin,0x08000000 -auto -startapp -exit
