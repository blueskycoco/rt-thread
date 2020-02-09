#! /bin/bash -e
#make clean
#make
'C:/Program Files (x86)/SEGGER/JLinkARM_V484f'/jflasharm -openprj$1 -openrtthread.bin,0x08000000 -auto -startapp -exit
