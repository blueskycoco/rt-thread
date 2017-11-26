#ifndef RT_CONFIG_H__
#define RT_CONFIG_H__

/* Automatically generated file; DO NOT EDIT. */
/* RT-Thread Configuration */

/* RT-Thread Kernel */

#define RT_NAME_MAX 8
#define RT_ALIGN_SIZE 4
#define RT_THREAD_PRIORITY_MAX 32
#define RT_TICK_PER_SECOND 100
#define RT_DEBUG
#define RT_USING_OVERFLOW_CHECK
#define RT_DEBUG_INIT 0
/* RT_DEBUG_THREAD is not set */
#define RT_USING_HOOK
#define IDLE_THREAD_STACK_SIZE 256
#define RT_USING_TIMER_SOFT
#define RT_TIMER_THREAD_PRIO 4
#define RT_TIMER_THREAD_STACK_SIZE 512

/* Inter-Thread communication */

#define RT_USING_SEMAPHORE
#define RT_USING_MUTEX
#define RT_USING_EVENT
#define RT_USING_MAILBOX
#define RT_USING_MESSAGEQUEUE
/* RT_USING_SIGNALS is not set */

/* Memory Management */

#define RT_USING_MEMPOOL
#define RT_USING_MEMHEAP
#define RT_USING_HEAP
#define RT_USING_SMALL_MEM
/* RT_USING_SLAB is not set */

/* Kernel Device Object */

#define RT_USING_DEVICE
// <bool name=RT_USING_DEVICE_IPC description="Using IPC in Device Driver Framework" default="true" />
#define RT_USING_DEVICE_IPC
// <bool name="RT_USING_SERIAL" description="Using Serial Device Driver Framework" default="true" />
#define RT_USING_SERIAL
// <integer name="RT_UART_RX_BUFFER_SIZE" description="The buffer size for UART reception" default="64" />
#define RT_UART_RX_BUFFER_SIZE    256
// <bool name="RT_USING_MTD_NAND" description="Using MTD NAND Framework" default="true" />
//#define RT_USING_MTD_NAND
// <bool name="RT_MTD_NAND_DEBUG" description="Enable MTD NAND Framework Debug" default="true" />
//#define RT_MTD_NAND_DEBUG
// <bool name="RT_USING_NFTL" description="Using NFTL layer" default="true" />
//#define RT_USING_NFTL
// <bool name="RT_USING_SPI" description="Using SPI Device Driver Framework" default="true" />
//#define RT_USING_SPI
// <bool name="RT_USING_I2C" description="Using I2C Device Driver Framework" default="true" />
//#define RT_USING_I2C
// <bool name="RT_USING_RTC" description="Using RTC Device Driver Framework" default="true" />
//#define RT_USING_RTC
// <integer name="RT_MMCSD_THREAD_PREORITY" description="The prority of mmcsd thread" default="15" />
#define RT_MMCSD_THREAD_PREORITY    15
// <section name="RT_USING_CONSOLE" description="Using console" default="true" >
//#define RT_USING_CONSOLE
// <integer name="RT_CONSOLEBUF_SIZE" description="The buffer size for console output" default="128" />
#define RT_CONSOLEBUF_SIZE  128
// <string name="RT_CONSOLE_DEVICE_NAME" description="The device name for console" default="uart" />
#define RT_CONSOLE_DEVICE_NAME  "uart0"
// </section>

#define RT_USING_COMPONENTS_INIT
#define RT_USING_USER_MAIN

/* C++ features */

/* RT_USING_CPLUSPLUS is not set */

/* Command shell */

#define RT_USING_FINSH
#define FINSH_USING_HISTORY
#define FINSH_USING_SYMTAB
#define FINSH_USING_DESCRIPTION
#define FINSH_THREAD_PRIORITY 20
#define FINSH_THREAD_STACK_SIZE 4096
#define FINSH_CMD_SIZE 80
/* FINSH_USING_AUTH is not set */
/* FINSH_USING_MSH is not set */

/* Device virtual file system */

/* RT_USING_DFS is not set */

/* Device Drivers */

#define RT_USING_DEVICE_IPC
#define RT_USING_SERIAL
/* RT_USING_CAN is not set */
/* RT_USING_HWTIMER is not set */
/* RT_USING_I2C is not set */
/* RT_USING_PIN is not set */
/* RT_USING_MTD_NOR is not set */
/* RT_USING_MTD_NAND is not set */
/* RT_USING_RTC is not set */
/* RT_USING_SDIO is not set */
/* RT_USING_SPI is not set */
/* RT_USING_WDT is not set */
/* RT_USING_USB_HOST is not set */
/* RT_USING_USB_DEVICE is not set */

/* POSIX layer and C standard library */

/* RT_USING_LIBC is not set */
/* RT_USING_PTHREADS is not set */

/* Network stack */

/* light weight TCP/IP stack */

#define RT_USING_LWIP
#define RT_USING_LWIP141
/* RT_USING_LWIP202 is not set */
/* RT_LWIP_IGMP is not set */
#define RT_LWIP_ICMP
/* RT_LWIP_SNMP is not set */
#define RT_LWIP_DNS
#define RT_LWIP_DHCP
#define IP_SOF_BROADCAST 1
#define IP_SOF_BROADCAST_RECV 1
/* LWIP_USING_DHCPD is not set */

/* Static IPv4 Address */

#define RT_LWIP_IPADDR "192.168.1.30"
#define RT_LWIP_GWADDR "192.168.1.1"
#define RT_LWIP_MSKADDR "255.255.255.0"
#define RT_LWIP_UDP
#define RT_LWIP_TCP
// <bool name="RT_LWIP_DNS" description="Enable DNS protocol" default="true" />
#define RT_LWIP_DNS
// <integer name="RT_LWIP_PBUF_NUM" description="Maximal number of buffers in the pbuf pool" default="4" />
#define RT_LWIP_PBUF_NUM    36
// <integer name="RT_LWIP_TCP_PCB_NUM" description="Maximal number of simultaneously active TCP connections" default="5" />
#define RT_LWIP_TCP_PCB_NUM 9
// <integer name="RT_LWIP_TCP_SND_BUF" description="TCP sender buffer size" default="8192" />
#define RT_LWIP_TCP_SND_BUF 8192 //4096
// <integer name="RT_LWIP_TCP_WND" description="TCP receive window" default="8192" />
#define RT_LWIP_TCP_WND 4096//2048
// <bool name="RT_LWIP_SNMP" description="Enable SNMP protocol" default="false" />
// #define RT_LWIP_SNMP
// <bool name="RT_LWIP_DHCP" description="Enable DHCP client to get IP address" default="false" />
//#define RT_LWIP_DHCP
// <integer name="RT_LWIP_TCP_SEG_NUM" description="the number of simultaneously queued TCP" default="4" />
#define RT_LWIP_TCP_SEG_NUM 144
// <integer name="RT_LWIP_TCPTHREAD_PRIORITY" description="the thread priority of TCP thread" default="128" />
#define RT_LWIP_TCPTHREAD_PRIORITY  12
// <integer name="RT_LWIP_TCPTHREAD_MBOX_SIZE" description="the mail box size of TCP thread to wait for" default="32" />
#define RT_LWIP_TCPTHREAD_MBOX_SIZE 8
// <integer name="RT_LWIP_TCPTHREAD_STACKSIZE" description="the thread stack size of TCP thread" default="4096" />
#define RT_LWIP_TCPTHREAD_STACKSIZE 4096
// <integer name="RT_LWIP_ETHTHREAD_PRIORITY" description="the thread priority of ethnetif thread" default="144" />
#define RT_LWIP_ETHTHREAD_PRIORITY  8
// <integer name="RT_LWIP_ETHTHREAD_MBOX_SIZE" description="the mail box size of ethnetif thread to wait for" default="8" />
#define RT_LWIP_ETHTHREAD_MBOX_SIZE 8
// <integer name="RT_LWIP_ETHTHREAD_STACKSIZE" description="the stack size of ethnetif thread" default="512" />
#define RT_LWIP_ETHTHREAD_STACKSIZE 512
// <ipaddr name="RT_LWIP_IPADDR" description="IP address of device" default="192.168.1.30" />
#define RT_LWIP_IPADDR0 192
#define RT_LWIP_IPADDR1 168
#define RT_LWIP_IPADDR2 1
#define RT_LWIP_IPADDR3 30
// <ipaddr name="RT_LWIP_GWADDR" description="Gateway address of device" default="192.168.1.1" />
#define RT_LWIP_GWADDR0 192
#define RT_LWIP_GWADDR1 168
#define RT_LWIP_GWADDR2 1
#define RT_LWIP_GWADDR3 1
// <ipaddr name="RT_LWIP_MSKADDR" description="Mask address of device" default="255.255.255.0" />
#define RT_LWIP_MSKADDR0 255
#define RT_LWIP_MSKADDR1 255
#define RT_LWIP_MSKADDR2 255
#define RT_LWIP_MSKADDR3 0

// </section>
#define RT_LWIP_IPV6 1
#define RT_LWIP_IPV6_AUTOCONFIG 1
#define RT_USING_LWIP_HEAD 1
#define RT_USING_NETUTILS 1
#define LWIP_CHECKSUM_ON_COPY 0
#define CHECKSUM_GEN_UDP 0
#define CHECKSUM_GEN_TCP 0
#define CHECKSUM_GEN_ICMP 0
#define CHECKSUM_GEN_ICMP6 0
#define CHECKSUM_GEN_IP 0
#define CHECKSUM_CHECK_IP 0
#define CHECKSUM_CHECK_UDP 0
#define CHECKSUM_CHECK_TCP 0
#define CHECKSUM_CHECK_ICMP 0
#define CHECKSUM_CHECK_ICMP6 0
#define RT_SERIAL_RB_BUFSZ 2048
//#define RT_LWIP_DHCP 1

// </RDTConfigurator>

#define RT_USING_UART0

#endif
