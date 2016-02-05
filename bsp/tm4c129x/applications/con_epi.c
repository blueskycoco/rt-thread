#include <rthw.h>
#include <rtthread.h>
#include <rtdevice.h>
#include "board.h"
#include "inc/hw_epi.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "driverlib/epi.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "driverlib/rom_map.h"
#include "driverlib/udma.h"
#include "con_socket.h"
extern struct rt_semaphore rx_sem[4];
struct rt_semaphore udma_sem;
struct rt_semaphore int_sem;
struct rt_mutex mutex;
extern bool phy_link;
int ack_got=0;
int cur_len=0;
void InitSWTransfer(uint32_t dest,uint32_t src,uint32_t len);
uint8_t pui8ControlTable[1024] __attribute__ ((aligned(1024)));

//*****************************************************************************
//
//! \addtogroup epi_examples_list
//! <h1>EPI SRAM Mode (sdram)</h1>
//!
//! This example shows how to configure the TM4C129 EPI bus in SRAM mode.  It
//! assumes that a 64Mbit SRAM is attached to EPI0.
//!
//! For the EPI SRAM mode, the pinout is as follows:
//!     Address11:0 - EPI0S11:0
//!     Bank1:0     - EPI0S14:13
//!     Data15:0    - EPI0S15:0
//!     DQML        - EPI0S16
//!     DQMH        - EPI0S17
//!     /CAS        - EPI0S18
//!     /RAS        - EPI0S19
//!     /WE         - EPI0S28
//!     /CS         - EPI0S29
//!     SDCKE       - EPI0S30
//!     SDCLK       - EPI0S31
//!
//! This example uses the following peripherals and I/O signals.  You must
//! review these and change as needed for your own board:
//! - EPI0 peripheral
//! - GPIO Port A peripheral (for EPI0 pins)
//! - GPIO Port B peripheral (for EPI0 pins)
//! - GPIO Port C peripheral (for EPI0 pins)
//! - GPIO Port G peripheral (for EPI0 pins)
//! - GPIO Port K peripheral (for EPI0 pins)
//! - GPIO Port L peripheral (for EPI0 pins)
//! - GPIO Port M peripheral (for EPI0 pins)
//! - GPIO Port N peripheral (for EPI0 pins)
//! - EPI0S0  - PK0
//! - EPI0S1  - PK1
//! - EPI0S2  - PK2
//! - EPI0S3  - PK3
//! - EPI0S4  - PC7
//! - EPI0S5  - PC6
//! - EPI0S6  - PC5
//! - EPI0S7  - PC4
//! - EPI0S8  - PA6
//! - EPI0S9  - PA7
//! - EPI0S10 - PG1
//! - EPI0S11 - PG0
//! - EPI0S12 - PM3
//! - EPI0S13 - PM2
//! - EPI0S14 - PM1
//! - EPI0S15 - PM0
//! - EPI0S16 - PL0
//! - EPI0S17 - PL1
//! - EPI0S18 - PL2
//! - EPI0S19 - PL3
//! - EPI0S28 - PB3
//! - EPI0S29 - PN2
//! - EPI0S30 - PN3
//! - EPI0S31 - PK5
//!
//! The following UART signals are configured only for displaying console
//! messages for this example.  These are not required for operation of EPI0.
//! - UART0 peripheral
//! - GPIO Port A peripheral (for UART0 pins)
//! - UART0RX - PA0
//! - UART0TX - PA1
//!
//! This example uses the following interrupt handlers.  To use this example
//! in your own application you must add these interrupt handlers to your
//! vector table.
//! - None.
//
//*****************************************************************************

//*****************************************************************************
//
// Use the following to specify the GPIO pins used by the SRAM EPI bus.
//
//*****************************************************************************
#define EPI_PORTA_PINS (GPIO_PIN_7 | GPIO_PIN_6)
#define EPI_PORTB_PINS (GPIO_PIN_3)
#define EPI_PORTC_PINS (GPIO_PIN_7 | GPIO_PIN_6 | GPIO_PIN_5 | GPIO_PIN_4)
#define EPI_PORTG_PINS (GPIO_PIN_1 | GPIO_PIN_0)
#define EPI_PORTK_PINS (GPIO_PIN_4 | GPIO_PIN_3 | GPIO_PIN_2 | GPIO_PIN_1 | GPIO_PIN_0)
#define EPI_PORTL_PINS (GPIO_PIN_1 | GPIO_PIN_0)
#define EPI_PORTM_PINS (GPIO_PIN_3 | GPIO_PIN_2 | GPIO_PIN_1 | GPIO_PIN_0)
#define EPI_PORTN_PINS (GPIO_PIN_2 | GPIO_PIN_3)

//*****************************************************************************
//
// The starting and ending address for the 64MB SRAM chip (32Meg x 16bits) on
// the SRAM daughter board.
//
//*****************************************************************************
#define SRAM_START_ADDRESS 0x00000000
#define SRAM_END_ADDRESS   0x000003FF
#define SIGNAL_CONFIG_SOCKET0 	0x00
#define SIGNAL_CONFIG_SOCKET1 	0x01
#define SIGNAL_CONFIG_SOCKET2 	0x02
#define SIGNAL_CONFIG_SOCKET3 	0x03
#define SIGNAL_DATA_SOCKET0 	0x04
#define SIGNAL_DATA_SOCKET1 	0x05
#define SIGNAL_DATA_SOCKET2 	0x06
#define SIGNAL_DATA_SOCKET3 	0x07
#define ACK_SIGNAL				0xF0 //ack last signal ACK_SIGNAL|SIGNAL_WE_GOT
#define SIGNAL_CONFIG_WITH_IPV6 0xc0 //ipv6 length is long , 0xc0|0x00,0x01,0x02,0x03 etc.
#define SOCKET0_R_ADDR			0x00000000
#define SOCKET0_W_ADDR			0x0000007D
#define SOCKET1_R_ADDR			0x000000FA
#define SOCKET1_W_ADDR			0x00000177
#define SOCKET2_R_ADDR			0x000001F4
#define SOCKET2_W_ADDR			0x00000271
#define SOCKET3_R_ADDR			0x000002EE
#define SOCKET3_W_ADDR			0x0000036B
#define SOCKET0_R_LEN_REG		0x000003E8//for config data len or socket data len
#define SOCKET0_W_LEN_REG		0x000003E9
#define SOCKET1_R_LEN_REG		0x000003EA
#define SOCKET1_W_LEN_REG		0x000003EB
#define SOCKET2_R_LEN_REG		0x000003EC
#define SOCKET2_W_LEN_REG		0x000003ED
#define SOCKET3_R_LEN_REG		0x000003EE
#define SOCKET3_W_LEN_REG		0x000003EF
#define A_TO_B_SOURCE_REG		0x000003F0
#define B_TO_A_SOURCE_REG		0x000003F1	//for config or raw,which socket
#define A_TO_B_PKT_LEN0			0x000003F2
#define A_TO_B_PKT_LEN1			0x000003F3
#define B_TO_A_PKT_LEN0			0x000003F4	//for config or raw,which socket
#define B_TO_A_PKT_LEN1			0x000003F5
#define CONFIG_A_TO_B_ADDR		0x000003FC	//sw signal to sure read done.
#define CONFIG_B_TO_A_ADDR		0x000003FD	//sw signal to sure read done.
#define A_TO_B_SIGNAL	   		0x000003FF//tm4c129x is A
#define B_TO_A_SIGNAL	   		0x000003FE//stm32 is B
#define SOCKET_BUF_LEN			250
#define SIGNAL_DATA_IN			0x55
#define SIGNAL_ACK_RCV			0x33
rt_bool_t can_send=RT_FALSE;
rt_bool_t op_state=RT_FALSE;//send state
rt_uint8_t *to_socket[32]={RT_NULL};
int index_epi=0;
#define EPI_BUF_LEN 1040
/*
work flow:
stm32 write data or config to tm4c129x
1 b_to_a int raise(stm32 write B_TO_A_SIGNAL addr data with SIGNAL_CONFIG_SOCKET0 or SIGNAL_DATA_SOCKET0 etc.)
2 read B_TO_A_SIGNAL addr to find what is happen.
3 if SIGNAL_CONFIG_SOCKET0 ... 
	read SOCKET0_R_LEN_REG to get config length
	read CONFIG_ADDR length data to config socket0,... (if SIGNAL_CONFIG_SOCKET0|SIGNAL_CONFIG_WITH_IPV6 read SOCKET0_R_ADDR)
	write ACK_SIGNAL|SIGNAL_CONFIG_SOCKET0 to A_TO_B_SIGNAL to ack the last SIGNAL_CONFIG_SOCKET0 cmd
	stm32 get int and get data ACK_SIGNAL|SIGNAL_CONFIG_SOCKET0 , this whole config process is done.
4 if SIGNAL_DATA_SOCKET0 ...
	read SOCKET0_R_LEN_REG to get config length
	read SOCKET0_R_ADDR length data to config socket0,...
	write ACK_SIGNAL|SIGNAL_DATA_SOCKET0 to A_TO_B_SIGNAL to ack the last SIGNAL_DATA_SOCKET0 cmd
	stm32 get int and get data ACK_SIGNAL|SIGNAL_DATA_SOCKET0 , this whole data transfer process is done.

tm4c129x get network data to inform stm32
1 a_to_b int raise (tm32c129x write A_TO_B addr with SIGNAL_DATA_SOCKET0 etc .)
2 stm32 read A_TO_B_SIGNAL addr to find what is happen
3 if SIGNAL_DATA_SOCKET0 ...
   read SOCKET0_W_LEN_REG to get data length
   read SOCKET0_W_ADDR with data length last got
   write ACK_SIGNAL | SIGNAL_DATA_SOCKET0 to B_TO_A_SIGNAL addr to ack the last SIGNAL_DATA_SOCKET0
4 if SIGNAL_CONFIG_SOCKET0 ...
   read SOCKET0_W_LEN_REG to get data length
   read CONFIG_ADDR with data length last got (if SIGNAL_CONFIG_SOCKET0|SIGNAL_CONFIG_WITH_IPV6 got ,read SOCKET0_W_ADDR)
   write ACK_SIGNAL | SIGNAL_CONFIG_SOCKET0 to B_TO_A_SIGNAL addr to ack the last SIGNAL_DATA_SOCKET0   
*/
//*****************************************************************************
//
// The Mapping address space for the EPI SRAM.
//
//*****************************************************************************
#define SRAM_MAPPING_ADDRESS 0x60000000

//*****************************************************************************
//
// A pointer to the EPI memory aperture.  Note that g_pui8EPISdram is declared
// as volatile so the compiler should not optimize reads out of the image.
//
//*****************************************************************************
static volatile uint8_t *g_pui8EPISdram;
struct rt_memheap system_heap;

void sram_init(void)
{
    /* initialize the built-in SRAM as a memory heap */
    rt_memheap_init(&system_heap,
                    "system",
                    (void *)SRAM_MAPPING_ADDRESS,
                    1024);
}

void *sram_malloc(unsigned long size)
{
    return rt_memheap_alloc(&system_heap, size);
}
RTM_EXPORT(sram_malloc);

void sram_free(void *ptr)
{
    rt_memheap_free(ptr);
}
RTM_EXPORT(sram_free);

void *sram_realloc(void *ptr, unsigned long size)
{
    return rt_memheap_realloc(&system_heap, ptr, size);
}
RTM_EXPORT(sram_realloc);

//*****************************************************************************
//
// Configure EPI0 in SRAM mode.  The EPI memory space is setup using an a
// simple C array.  This example shows how to read and write to an SRAM card
// using the EPI bus in SRAM mode.
//
//*****************************************************************************
int epi_init(void)
{
    uint32_t ui32Val, ui32Freq,i;

    //
    // Display the setup on the console.
    //
    rt_kprintf("EPI DUAL SRAM Mode ->\n");
    rt_kprintf("  Type: SRAM\n");
    rt_kprintf("  Starting Address: 0x%08x\n", SRAM_MAPPING_ADDRESS);
    rt_kprintf("  End Address: 0x%08x\n",
               (SRAM_MAPPING_ADDRESS + SRAM_END_ADDRESS));
    rt_kprintf("  Data: 8-bit\n");
    rt_kprintf("  Size: 1KB (128 x 8bits)\n\n");

    //
    // The EPI0 peripheral must be enabled for use.
    //
    SysCtlPeripheralDisable(SYSCTL_PERIPH_EPI0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_EPI0);

    //
    // For this example EPI0 is used with multiple pins on PortA, B, C, G, H,
    // K, L, M and N.  The actual port and pins used may be different on your
    // part, consult the data sheet for more information.
    // TODO: Update based upon the EPI pin assignment on your target part.
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOG);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOK);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOL);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOM);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);
	//SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOQ);
	/*
	HOST 8 mode
	 EPI0S0 D0      PK0
	 EPI0S1 D1      PK1
	 EPI0S2 D2      PK2
	 EPI0S3 D3      PK3
	 EPI0S4 D4      PC7
	 EPI0S5 D5      PC6
	 EPI0S6 D6      PC5
	 EPI0S7 D7      PC4
	 EPI0S8 A0      PA6
	 EPI0S9 A1      PA7
	 EPI0S10 A2    PG1
	 EPI0S11 A3    PG0
	 EPI0S12 A4    PM3
	 EPI0S13 A5    PM2
	 EPI0S14 A6    PM1
	 EPI0S15 A7    PM0
	 EPI0S16 A8    PL0
	 EPI0S17 A9    PL1
	 EPI0S18 A10 
	 EPI0S19 A11 
	 EPI0S20 A12
	 EPI0S26 SEML PL4(just cy7c)
	 EPI0S28 OEL   PB3
	 EPI0S29 R/WL PN2
	 EPI0S30 CS0n PN3
	 EPI0S31 INTL  PK5
	 EPI0S32 BUSY PK4
	*/
	
	MAP_GPIOIntDisable(GPIO_PORTK_BASE, GPIO_PIN_5);
	MAP_GPIOPinTypeGPIOInput(GPIO_PORTK_BASE, GPIO_PIN_5);
	MAP_GPIOIntTypeSet(GPIO_PORTK_BASE, GPIO_PIN_5, GPIO_FALLING_EDGE);
	MAP_IntEnable(INT_GPIOK);
	MAP_GPIOIntEnable(GPIO_PORTK_BASE, GPIO_PIN_5);
	int ui32Status = MAP_GPIOIntStatus(GPIO_PORTK_BASE, true);
	MAP_GPIOIntClear(GPIO_PORTK_BASE, ui32Status);

    //
    // This step configures the internal pin muxes to set the EPI pins for use
    // with EPI.  Please refer to the datasheet for more information about pin
    // muxing.  Note that EPI0S27:20 are not used for the EPI SRAM
    // implementation.
    // TODO: Update this section based upon the EPI pin assignment on your
    // target part.
    //
    //
	MAP_GPIOPinConfigure(GPIO_PK0_EPI0S0);
	GPIOPinTypeEPI(GPIO_PORTK_BASE, GPIO_PIN_0);
	MAP_GPIOPinConfigure(GPIO_PK1_EPI0S1);
	GPIOPinTypeEPI(GPIO_PORTK_BASE, GPIO_PIN_1);
	MAP_GPIOPinConfigure(GPIO_PK2_EPI0S2);
	GPIOPinTypeEPI(GPIO_PORTK_BASE, GPIO_PIN_2);
	MAP_GPIOPinConfigure(GPIO_PK3_EPI0S3);
	GPIOPinTypeEPI(GPIO_PORTK_BASE, GPIO_PIN_3);
	MAP_GPIOPinConfigure(GPIO_PK4_EPI0S32);
	GPIOPinTypeEPI(GPIO_PORTK_BASE, GPIO_PIN_4);
	MAP_GPIOPinConfigure(GPIO_PC7_EPI0S4);
	GPIOPinTypeEPI(GPIO_PORTC_BASE, GPIO_PIN_7);
	MAP_GPIOPinConfigure(GPIO_PC6_EPI0S5);
	GPIOPinTypeEPI(GPIO_PORTC_BASE, GPIO_PIN_6);
	MAP_GPIOPinConfigure(GPIO_PC5_EPI0S6);
	GPIOPinTypeEPI(GPIO_PORTC_BASE, GPIO_PIN_5);
	MAP_GPIOPinConfigure(GPIO_PC4_EPI0S7);
	GPIOPinTypeEPI(GPIO_PORTC_BASE, GPIO_PIN_4);
	MAP_GPIOPinConfigure(GPIO_PA6_EPI0S8);
	GPIOPinTypeEPI(GPIO_PORTA_BASE, GPIO_PIN_6);
	MAP_GPIOPinConfigure(GPIO_PA7_EPI0S9);
	GPIOPinTypeEPI(GPIO_PORTA_BASE, GPIO_PIN_7);
	MAP_GPIOPinConfigure(GPIO_PG1_EPI0S10);
	GPIOPinTypeEPI(GPIO_PORTG_BASE, GPIO_PIN_1);
	MAP_GPIOPinConfigure(GPIO_PG0_EPI0S11);
	GPIOPinTypeEPI(GPIO_PORTG_BASE, GPIO_PIN_0);
	MAP_GPIOPinConfigure(GPIO_PM3_EPI0S12);
	GPIOPinTypeEPI(GPIO_PORTM_BASE, GPIO_PIN_3);
	MAP_GPIOPinConfigure(GPIO_PM2_EPI0S13);
	GPIOPinTypeEPI(GPIO_PORTM_BASE, GPIO_PIN_2);
	MAP_GPIOPinConfigure(GPIO_PM1_EPI0S14);
	GPIOPinTypeEPI(GPIO_PORTM_BASE, GPIO_PIN_1);
	MAP_GPIOPinConfigure(GPIO_PM0_EPI0S15);
	GPIOPinTypeEPI(GPIO_PORTM_BASE, GPIO_PIN_0);
	MAP_GPIOPinConfigure(GPIO_PL0_EPI0S16);
	GPIOPinTypeEPI(GPIO_PORTL_BASE, GPIO_PIN_0);
	MAP_GPIOPinConfigure(GPIO_PL1_EPI0S17);
	GPIOPinTypeEPI(GPIO_PORTL_BASE, GPIO_PIN_1);
	MAP_GPIOPinConfigure(GPIO_PB3_EPI0S28);
	GPIOPinTypeEPI(GPIO_PORTB_BASE, GPIO_PIN_3);
	MAP_GPIOPinConfigure(GPIO_PN2_EPI0S29);
	GPIOPinTypeEPI(GPIO_PORTN_BASE, GPIO_PIN_2);
	MAP_GPIOPinConfigure(GPIO_PN3_EPI0S30);
	GPIOPinTypeEPI(GPIO_PORTN_BASE, GPIO_PIN_3);
    //
    // Is our current system clock faster than we can drive the SRAM clock?
    //
    if(SysClock > 60000000)
    {
        //
        // Yes. Set the EPI clock to half the system clock.
        //
        EPIDividerSet(EPI0_BASE, 1);
    }
    else
    {
        //
        // With a system clock of 60MHz or lower, we can drive the SRAM at
        // the same rate so set the divider to 0.
        //
        EPIDividerSet(EPI0_BASE, 0);
    }

    //
    // Sets the usage mode of the EPI module.  For this example we will use
    // the SRAM mode to talk to the external 64MB SRAM daughter card.
    //
    EPIModeSet(EPI0_BASE, EPI_MODE_HB8);
	EPIConfigHB8Set(EPI0_BASE, (EPI_HB8_MODE_SRAM| EPI_HB8_CSCFG_CS|EPI_HB8_IN_READY_EN),0);

    //
    // Set the address map.  The EPI0 is mapped from 0x60000000 to 0x01FFFFFF.
    // For this example, we will start from a base address of 0x60000000 with
    // a size of 256MB.  Although our SRAM is only 64MB, there is no 64MB
    // aperture option so we pick the next larger size.
    //
    EPIAddressMapSet(EPI0_BASE, EPI_ADDR_RAM_SIZE_64KB | EPI_ADDR_RAM_BASE_6);

    //
    // Wait for the SRAM wake-up to complete by polling the SRAM
    // initialization sequence bit.  This bit is true when the SRAM interface
    // is going through the initialization and false when the SRAM interface
    // it is not in a wake-up period.
    //
    while(HWREG(EPI0_BASE + EPI_O_STAT) &  EPI_STAT_INITSEQ)
    {
    }
    //
    // Set the EPI memory pointer to the base of EPI memory space.  Note that
    // g_pui8EPISdram is declared as volatile so the compiler should not
    // optimize reads out of the memory.  With this pointer, the memory space
    // is accessed like a simple array.
    //
    g_pui8EPISdram = (uint8_t *)0x60000000;

    //
    // Read the initial data in SRAM, and display it on the console.
    //
    rt_kprintf("  SRAM Initial Data:\n");
    rt_kprintf("     Mem[0x6000.0000] = 0x%02x\n",
               g_pui8EPISdram[SRAM_START_ADDRESS]);
    rt_kprintf("     Mem[0x6000.0001] = 0x%02x\n",
               g_pui8EPISdram[SRAM_START_ADDRESS + 1]);
    rt_kprintf("     Mem[0x6000.03FE] = 0x%02x\n",
               g_pui8EPISdram[SRAM_END_ADDRESS - 3]);
    rt_kprintf("     Mem[0x6000.03FF] = 0x%02x\n\n",
               g_pui8EPISdram[SRAM_END_ADDRESS-2]);

    //
    // Display what writes we are doing on the console.
    //
    rt_kprintf("  SRAM Write:\n");
    rt_kprintf("     Mem[0x6000.0000] <- 0xab\n");
    rt_kprintf("     Mem[0x6000.0001] <- 0x12\n");
    rt_kprintf("     Mem[0x6000.03FE] <- 0xdc\n");
    rt_kprintf("     Mem[0x6000.03FF] <- 0x2a\n\n");

    //
    // Write to the first 2 and last 2 address of the SRAM card.  Since the
    // SRAM card is word addressable, we will write words.
    //
    g_pui8EPISdram[SRAM_START_ADDRESS] = 0xab;
    g_pui8EPISdram[SRAM_START_ADDRESS + 1] = 0x12;
    g_pui8EPISdram[SRAM_END_ADDRESS - 3] = 0xdc;
    g_pui8EPISdram[SRAM_END_ADDRESS-2] = 0x2a;
    //
    // Read back the data you wrote, and display it on the console.
    //
    rt_kprintf("  SRAM Read:\n");
    rt_kprintf("     Mem[0x6000.0000] = 0x%02x\n",
              g_pui8EPISdram[SRAM_START_ADDRESS]);
    rt_kprintf("     Mem[0x6000.0001] = 0x%02x\n",
               g_pui8EPISdram[SRAM_START_ADDRESS + 1]);
    rt_kprintf("     Mem[0x6000.03FE] = 0x%02x\n",
               g_pui8EPISdram[SRAM_END_ADDRESS - 3]);
    rt_kprintf("     Mem[0x6000.03FF] = 0x%02x\n\n",
               g_pui8EPISdram[SRAM_END_ADDRESS-2]);
    //
    // Read and write operations were successful.  Return with no errors.
    //
    rt_kprintf("Read and write to external SRAM was successful!\n");
	for(i=0;i<256;i++)
		g_pui8EPISdram[i]=i;
	for(i=256;i<512;i++)
		g_pui8EPISdram[i]=i-256;
	for(i=512;i<768;i++)
		g_pui8EPISdram[i]=i-512;
	for(i=768;i<1022;i++)
		g_pui8EPISdram[i]=i-768;
	for(i=0;i<1022;i++)
	{
		if(g_pui8EPISdram[i]!=(i%256))
			rt_kprintf("<%02x> %d failed\n",g_pui8EPISdram[i],i%256);
		rt_kprintf("0x%02x ",g_pui8EPISdram[i]);			
		if(((i+1)%16)==0)
			rt_kprintf("\n");
		if(((i+1)%256)==0)
			rt_kprintf("\n");
	}
	rt_kprintf("\n");
	rt_mutex_init(&mutex, "epimutex", RT_IPC_FLAG_FIFO);
	//#if A_TO_B
	//if(g_pui8EPISdram[B_TO_A_SIGNAL]==0x55)
	//	g_pui8EPISdram[CONFIG_B_TO_A_ADDR]=0xff;
	//#else
	//if(g_pui8EPISdram[A_TO_B_SIGNAL]==0x55)
	//	g_pui8EPISdram[CONFIG_A_TO_B_ADDR]=0xff;
	//#endif
	//
	// Enable the uDMA controller at the system level.	Enable it to continue
	// to run while the processor is in sleep.
	//
	MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_UDMA);
	MAP_SysCtlPeripheralSleepEnable(SYSCTL_PERIPH_UDMA);
	
	//
	// Enable the uDMA controller error interrupt.	This interrupt will occur
	// if there is a bus error during a transfer.
	//
	MAP_IntEnable(INT_UDMAERR);
	
	//
	// Enable the uDMA controller.
	//
	MAP_uDMAEnable();
	
	//
	// Point at the control table to use for channel control structures.
	//
	MAP_uDMAControlBaseSet(pui8ControlTable);
	rt_sem_init(&udma_sem, "udma_sem", 0, 0);
	rt_sem_init(&int_sem, "int_sem", 0, 0);
    return(0);	
}
void IntGpioK()
{
	int i=0;
	if(MAP_GPIOIntStatus(GPIO_PORTK_BASE, true)&GPIO_PIN_5)
	{
		MAP_GPIOIntClear(GPIO_PORTK_BASE, GPIO_PIN_5);
		#if A_TO_B
		if(g_pui8EPISdram[B_TO_A_SIGNAL]==0 && g_pui8EPISdram[A_TO_B_SIGNAL]==0)
		{
			//rt_kprintf("B has read data done.\n");
			rt_sem_release(&(int_sem));
		}
		#else
		rt_sem_release(&(rx_sem[0]));
		#endif
	}
}
void Signal_To_B(int len)
{
	//write packet len to 0x3fe high , 0x3ff low
	g_pui8EPISdram[B_TO_A_SIGNAL]=(len>>8)&0xff;
	g_pui8EPISdram[A_TO_B_SIGNAL]=len&0xff;
	//rt_kprintf("Signale to B ,len %d\n",len);
	rt_sem_take(&int_sem, RT_WAITING_FOREVER);
}
void Signal_To_A(unsigned char data,int len)
{
	g_pui8EPISdram[B_TO_A_PKT_LEN0]=len&0xff;
	g_pui8EPISdram[B_TO_A_PKT_LEN1]=(len>>8)&0xff;
	g_pui8EPISdram[CONFIG_B_TO_A_ADDR]=0x00;
	g_pui8EPISdram[B_TO_A_SIGNAL]=data;
	while(g_pui8EPISdram[CONFIG_B_TO_A_ADDR]==0x00)
		rt_thread_delay(1);
}
void Write_A_B(unsigned char begin)
{
	memset(g_pui8EPISdram,begin,510);
}
void Write_B_A(unsigned char begin,int len)
{
	memset(g_pui8EPISdram+510,begin,len);
}
int _epi_write(int index, const void *buffer, int size,unsigned char signal)
{	
#if 1
	int len=0;
	//rt_kprintf("epi_write len %d\n",size);
	if(size<=1022)
	{
		memcpy(g_pui8EPISdram,buffer,size);
		//InitSWTransfer(g_pui8EPISdram,buffer,(size/4));
		//if(size%4!=0)
		//	rt_memcpy(g_pui8EPISdram+(size/4)*4,buffer+(size/4)*4,size-(size/4)*4);
		Signal_To_B(size);
	}
	else
	{
		while(len!=size)
		{
			if((len+1022)<size)
			{
				memcpy(g_pui8EPISdram,buffer+len,1022);
				//InitSWTransfer(g_pui8EPISdram,buffer,(1020/4));
				//rt_memcpy(g_pui8EPISdram+1020,buffer+1020,2);
				Signal_To_B(1022);
				len=len+1022;
			}
			else
			{
				memcpy(g_pui8EPISdram,buffer+len,size-len);
				//InitSWTransfer(g_pui8EPISdram,buffer+len,((size-len)/4));
				//if((size-len)%4!=0)
				//	rt_memcpy(g_pui8EPISdram+((size-len)/4)*4,buffer+len+((size-len)/4)*4,size-len-((size-len)/4)*4);
				Signal_To_B(size-len);
				len+=size-len;
			}
		}
	}
#else
	static int all_len=0;
	int i;
	all_len+=size;
	cur_len=size;
	//rt_kprintf("Total send B_TO_A %d==>%d\n",all_len,size);
	//for(i=0;i<size;i++)
	//	rt_kprintf("%c",((char *)buffer)[i]);
	//rt_kprintf("==>\n");
	int len=0;
	if(size<=498)
	{
		memcpy(g_pui8EPISdram+510,buffer,size);
		#if 0
		InitSWTransfer(g_pui8EPISdram+510,buffer,(size/4));
		if(size%4!=0)
			rt_memcpy(g_pui8EPISdram+510+(size/4)*4,buffer+(size/4)*4,size-(size/4)*4);
		#endif
		//for(i=0;i<size;i++)
		//rt_kprintf("%c",((char *)g_pui8EPISdram)[510+i]);
		Signal_To_A(0x55,size);
	}
	else
	{
		while(len!=size)
		{
			if((len+498)<size)
			{
				//rt_kprintf("send B_TO_A 498\n");
				memcpy(g_pui8EPISdram+510,buffer+len,498);
				//InitSWTransfer(g_pui8EPISdram+510,buffer,496/4);
				//rt_memcpy(g_pui8EPISdram+510+496,buffer+496,2);
				Signal_To_A(0x55,498);
				len=len+498;
			}
			else
			{
				//rt_kprintf("send B_TO_A %d\n",size-len);
				memcpy(g_pui8EPISdram+510,buffer+len,size-len);
				//InitSWTransfer(g_pui8EPISdram+510,buffer+len,(size-len)/4);
				//if((size-len)%4!=0)
				//	rt_memcpy(g_pui8EPISdram+510+((size-len)/4)*4+1,buffer+((size-len)/4)*4,size-len-((size-len)/4)*4);
				Signal_To_A(0x55,size-len);
				len+=size-len;
			}
		}
	}
	//rt_kprintf("send done.\n");	
	#endif
	return 0;	
}

void _epi_read()
{	

	unsigned char *buf=RT_NULL;
	int index_len=0,i;
	char cnt=0;
	unsigned char *index_addr;
	int do_config=0;
	int b_to_a_len=0;
	static int all_len=0;
	#if !A_TO_B
	#if 1
	rt_uint8_t *buf1;
	int a_to_b_len=0;
	a_to_b_len=g_pui8EPISdram[B_TO_A_SIGNAL]<<8|g_pui8EPISdram[A_TO_B_SIGNAL];
	//rt_kprintf("a_to_b_len %d\n",a_to_b_len);
	unsigned char source=g_pui8EPISdram[A_TO_B_SIGNAL];
	//if(source==SIGNAL_DATA_IN)
	{		
		if(phy_link&&g_socket[0].connected)
		{
			#if 1
			buf1 =(rt_uint8_t *)malloc(a_to_b_len*sizeof(rt_uint8_t));
			if(buf1==RT_NULL)
				rt_kprintf("buf is RT_NULL\r\n");
			else
			{
				rt_memcpy(buf1,g_pui8EPISdram,a_to_b_len);
				rt_data_queue_push(&g_data_queue[0],buf1, a_to_b_len, RT_WAITING_FOREVER);
			}
			#else
			if(to_socket[index_epi]==RT_NULL)
			{
				to_socket[index_epi]=(unsigned char *)malloc(1000*6*sizeof(unsigned char));
				if(to_socket[index_epi]==RT_NULL)
					rt_kprintf("to_socket is RT_NULL\r\n");
			}
			if(all_len+a_to_b_len<6000)
			{
				rt_memcpy(to_socket[index_epi]+all_len,g_pui8EPISdram,a_to_b_len);
				all_len+=a_to_b_len;
			}
			else
			{
				rt_data_queue_push(&g_data_queue[0],to_socket[index_epi], all_len, RT_WAITING_FOREVER);
				if(index_epi<31)
					index_epi++;
				else
					index_epi=0;
				to_socket[index_epi]=(unsigned char *)malloc(1000*6*sizeof(unsigned char));
				if(to_socket[index_epi]==RT_NULL)
					rt_kprintf("to_socket is RT_NULL2\r\n");
				else
					rt_memcpy(to_socket[index_epi],g_pui8EPISdram,a_to_b_len);				
				all_len=a_to_b_len;
			}
			#endif
		}
		g_pui8EPISdram[A_TO_B_SIGNAL]=0x00;
		g_pui8EPISdram[B_TO_A_SIGNAL]=0x00;
	}
	#else
	unsigned char local_buf[1440]={0};
	static int packet_len=0,send_len=0;
	int a_to_b_len=0;
	a_to_b_len=g_pui8EPISdram[A_TO_B_PKT_LEN1]<<8|g_pui8EPISdram[A_TO_B_PKT_LEN0];
	all_len+=a_to_b_len;
	unsigned char source=g_pui8EPISdram[A_TO_B_SIGNAL];
	//rt_kprintf("\nGOT A_TO_B Data %d==>%d\n",all_len,a_to_b_len);
	if(source==SIGNAL_DATA_IN)
	{
		if(to_socket[index_epi]==RT_NULL)
		{
			to_socket[index_epi]=(unsigned char *)malloc(1440*6*sizeof(unsigned char));
			if(to_socket[index_epi]==RT_NULL)
				rt_kprintf("to_socket is RT_NULL\r\n");
			//else
			//	rt_kprintf("cur_len %d==>%d\n",cur_len,index_epi);
		}
		if(a_to_b_len==444)
		{
			if(send_len+a_to_b_len==1440*6)
			can_send=RT_TRUE; 
		}
		else if(a_to_b_len!=498)
			can_send=RT_TRUE;
		//rt_memcpy(to_socket[index_epi]+send_len,g_pui8EPISdram,a_to_b_len);
		//if(a_to_b_len>=8)
		//{
		//	InitSWTransfer(to_socket[index_epi]+send_len,g_pui8EPISdram,a_to_b_len/4);
		//	if(a_to_b_len%4!=0)
		//		rt_memcpy(to_socket[index_epi]+send_len+(a_to_b_len/4)*4,g_pui8EPISdram+(a_to_b_len/4)*4,a_to_b_len-(a_to_b_len/4)*4);
		//}
		//else
			rt_memcpy(to_socket[index_epi]+send_len,g_pui8EPISdram,a_to_b_len);
		send_len+=a_to_b_len;
		
		if(can_send)
		{
			if(phy_link&&g_socket[0].connected)
			{
				rt_data_queue_push(&g_data_queue[0],to_socket[index_epi], send_len, RT_WAITING_FOREVER);
				//rt_kprintf("\nGOT A_TO_B Data %d==>%d\n",send_len,a_to_b_len);
				//rt_kprintf("push len %d\n",send_len);
				//int i;
				//for(i=0;i<send_len;i++)
				//	rt_kprintf("%c",to_socket[index_epi][i]);
			}
			else
			{
				rt_free(to_socket[index_epi]);
				to_socket[index_epi]=RT_NULL;
			}
			if(index_epi<31)
				index_epi++;
			else
				index_epi=0;
			to_socket[index_epi]=(unsigned char *)malloc(1440*6*sizeof(unsigned char));
			if(to_socket[index_epi]==RT_NULL)
				rt_kprintf("to_socket is RT_NULL2\r\n");
			//else
			//	rt_kprintf("cur_l2en %d==>%d\n",cur_len,index_epi);
			
			send_len=0;			
			can_send=RT_FALSE;
		}		
		g_pui8EPISdram[CONFIG_A_TO_B_ADDR]=0xff;		
	}
	#endif
	#else
	unsigned char source=g_pui8EPISdram[B_TO_A_SIGNAL];
	//rt_kprintf("\n_epi_read source %02x\r\n",source);
	if(source==SIGNAL_DATA_IN)
	{	
		b_to_a_len=(g_pui8EPISdram[B_TO_A_PKT_LEN1]<<8)|g_pui8EPISdram[B_TO_A_PKT_LEN0];
		all_len+=b_to_a_len;
		//rt_kprintf("\nGOT B_TO_A Data %d\n",all_len);
		//for(i=510;i<1019;i++)
		//	rt_kprintf("%d ",g_pui8EPISdram[i]);
		g_pui8EPISdram[CONFIG_B_TO_A_ADDR]=0xff;
		#if A_PLACE
		memcpy(g_pui8EPISdram,g_pui8EPISdram+510,b_to_a_len);
		#if 0
		InitSWTransfer(g_pui8EPISdram,g_pui8EPISdram+510,b_to_a_len/4);
		if(b_to_a_len%4!=0)
			rt_memcpy(g_pui8EPISdram+(b_to_a_len/4)*4,g_pui8EPISdram+510+(b_to_a_len/4)*4,b_to_a_len-(b_to_a_len/4)*4);
		#endif
		Signal_To_B(b_to_a_len);
		#endif
	}
	#endif
	return ;	
}
void uDMAErrorHandler(void)
{
    uint32_t ui32Status;

    //
    // Check for uDMA error bit
    //
    ui32Status = MAP_uDMAErrorStatusGet();

    //
    // If there is a uDMA error, then clear the error and increment
    // the error counter.
    //
    if(ui32Status)
    {
        MAP_uDMAErrorStatusClear();
        //g_ui32uDMAErrCount++;
        rt_kprintf("uDMA Error intr.\n");
    }
}

void uDMAIntHandler(void)
{
    uint32_t ui32Mode;

    //
    // Check for the primary control structure to indicate complete.
    //
    ui32Mode = MAP_uDMAChannelModeGet(UDMA_CHANNEL_SW);
    if(ui32Mode == UDMA_MODE_STOP)
    {
        //
        // Increment the count of completed transfers.
        //
        //g_ui32MemXferCount++;

        //
        // Configure it for another transfer.
        //
        //ROM_uDMAChannelTransferSet(UDMA_CHANNEL_SW, UDMA_MODE_AUTO,
        //                           g_ui32SrcBuf, g_ui32DstBuf,
        //                           MEM_BUFFER_SIZE);

        //
        // Initiate another transfer.
        //
        //ROM_uDMAChannelEnable(UDMA_CHANNEL_SW);
        //ROM_uDMAChannelRequest(UDMA_CHANNEL_SW);
        //rt_kprintf("uDMA done.\n");
        rt_sem_release(&udma_sem);
    }

    //
    // If the channel is not stopped, then something is wrong.
    //
    else
    {
        //g_ui32BadISR++;
        rt_kprintf("uDMA Bad intr.\n");
    }
}
void InitSWTransfer(uint32_t dest,uint32_t src,uint32_t len)
{

    //
    // Enable interrupts from the uDMA software channel.
    //
    MAP_IntEnable(INT_UDMA);

    //
    // Put the attributes in a known state for the uDMA software channel.
    // These should already be disabled by default.
    //
    MAP_uDMAChannelAttributeDisable(UDMA_CHANNEL_SW,
                                    UDMA_ATTR_USEBURST | UDMA_ATTR_ALTSELECT |
                                    (UDMA_ATTR_HIGH_PRIORITY |
                                    UDMA_ATTR_REQMASK));

    //
    // Configure the control parameters for the SW channel.  The SW channel
    // will be used to transfer between two memory buffers, 32 bits at a time.
    // Therefore the data size is 32 bits, and the address increment is 32 bits
    // for both source and destination.  The arbitration size will be set to 8,
    // which causes the uDMA controller to rearbitrate after 8 items are
    // transferred.  This keeps this channel from hogging the uDMA controller
    // once the transfer is started, and allows other channels cycles if they
    // are higher priority.
    //
    MAP_uDMAChannelControlSet(UDMA_CHANNEL_SW | UDMA_PRI_SELECT,
                              UDMA_SIZE_32 | UDMA_SRC_INC_32 | UDMA_DST_INC_32 |
                              UDMA_ARB_8);

    //
    // Set up the transfer parameters for the software channel.  This will
    // configure the transfer buffers and the transfer size.  Auto mode must be
    // used for software transfers.
    //
    MAP_uDMAChannelTransferSet(UDMA_CHANNEL_SW | UDMA_PRI_SELECT,
                               UDMA_MODE_AUTO, src, dest,
                               len);

    //
    // Now the software channel is primed to start a transfer.  The channel
    // must be enabled.  For software based transfers, a request must be
    // issued.  After this, the uDMA memory transfer begins.
    //
    MAP_uDMAChannelEnable(UDMA_CHANNEL_SW);
    MAP_uDMAChannelRequest(UDMA_CHANNEL_SW);
	rt_sem_take(&udma_sem, RT_WAITING_FOREVER);
}

