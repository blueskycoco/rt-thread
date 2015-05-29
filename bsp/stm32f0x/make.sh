#! /bin/bash
export RTT_EXEC_PATH="d:/Program Files/CodeSourcery/Sourcery_CodeBench_Lite_for_ARM_EABI/bin"
scons.bat
jlink.exe burn.txt
