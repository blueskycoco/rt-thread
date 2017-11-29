#! /bin/bash -e
scons.bat
jflash -openprjstm32f103zet6.jflash -openrtthread.bin,0x08000000 -auto -startapp -exit
