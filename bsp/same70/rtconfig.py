import os

# toolchains options
ARCH='arm'
CPU='cortex-m7'
CROSS_TOOL='gcc'

if os.getenv('RTT_CC'):
    CROSS_TOOL = os.getenv('RTT_CC')
if os.getenv('RTT_ROOT'):
    RTT_ROOT = os.getenv('RTT_ROOT')

PLATFORM 	= 'gcc'
EXEC_PATH 	= r'z:/gcc-arm-none-eabi-5_4-2016q3-20160926-win32/bin'
#EXEC_PATH      = r'e:/project/atmel/gcc-arm-none-eabi-5_4-2016q3-20160926-win32/bin'
BUILD = 'debug'

# toolchains
PREFIX = 'arm-none-eabi-'
CC = PREFIX + 'gcc'
CXX = PREFIX + 'g++'
AS = PREFIX + 'gcc'
AR = PREFIX + 'ar'
LINK = PREFIX + 'gcc'
TARGET_EXT = 'axf'
SIZE = PREFIX + 'size'
OBJDUMP = PREFIX + 'objdump'
OBJCPY = PREFIX + 'objcopy'
STRIP = PREFIX + 'strip'

DEVICE = '  -mcpu=cortex-m7 -mthumb -mfpu=fpv5-sp-d16 -mfloat-abi=softfp -ffunction-sections -fdata-sections'
CFLAGS = DEVICE + ' -g -Wall -DBOARD_SAME70_XPLD -D__SAME70Q21__ -DTRACE_LEVEL=4 -eentry'
AFLAGS = ' -c' + DEVICE + ' -x assembler-with-cpp -Wa,-mimplicit-it=thumb -D__ASSEMBLY__ '
LFLAGS = DEVICE + ' -lm -lgcc -lc' + ' -nostartfiles -Wl,--gc-sections,-Map=rtthread_same70.map,-cref,-u,Reset_Handler -T same70q21_flash.ld'

CPATH = ''
LPATH = ''

if BUILD == 'debug':
    CFLAGS += ' -O0 -gdwarf-2'
    AFLAGS += ' -gdwarf-2'
else:
    CFLAGS += ' -O2 -Os'

POST_ACTION = OBJCPY + ' -O binary $TARGET rtthread.bin\n' + SIZE + ' $TARGET \n'

# module setting 
CXXFLAGS = ' -Woverloaded-virtual -fno-exceptions -fno-rtti '
M_CFLAGS = CFLAGS + ' -mlong-calls -fPIC '
M_CXXFLAGS = CXXFLAGS + ' -mlong-calls -fPIC'
M_LFLAGS = DEVICE + CXXFLAGS + ' -Wl,--gc-sections,-z,max-page-size=0x4' +\
                                    ' -shared -fPIC -nostartfiles -static-libgcc'
M_POST_ACTION = STRIP + ' -R .hash $TARGET\n' + SIZE + ' $TARGET \n'

