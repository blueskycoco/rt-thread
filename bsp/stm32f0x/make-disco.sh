#! /bin/bash
#export RTT_EXEC_PATH="c:\Users\rdp\MentorGraphics\Sourcery_CodeBench_Lite_for_ARM_EABI\bin\"
 RTT_CC=gcc scons.bat
./MB786.bat
#openocd -f openocd.cfg -c "flash_image"
#openocd -f openocd.cfg -c "flash_param"
