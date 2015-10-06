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
#include "driverlib/rom_map.h"

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
#define A_TO_B_SIGNAL	   0x000003FF//tm4c129x is A
#define B_TO_A_SIGNAL	   0x000003FE//stm32 is B
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
	/*
	MAP_GPIOPinConfigure(GPIO_PL2_EPI0S18);
	GPIOPinTypeEPI(GPIO_PORTL_BASE, GPIO_PIN_2);
	MAP_GPIOPinConfigure(GPIO_PL3_EPI0S19);
	GPIOPinTypeEPI(GPIO_PORTL_BASE, GPIO_PIN_3);
	MAP_GPIOPinConfigure(GPIO_PL4_EPI0S26);
	GPIOPinTypeEPI(GPIO_PORTL_BASE, GPIO_PIN_4);
	MAP_GPIOPinConfigure(GPIO_PQ0_EPI0S20);
	GPIOPinTypeEPI(GPIO_PORTQ_BASE, GPIO_PIN_0);
	MAP_GPIOPinConfigure(GPIO_PQ1_EPI0S21);
	GPIOPinTypeEPI(GPIO_PORTQ_BASE, GPIO_PIN_1);	
	MAP_GPIOPinConfigure(GPIO_PQ2_EPI0S22);
	GPIOPinTypeEPI(GPIO_PORTQ_BASE, GPIO_PIN_2);
	MAP_GPIOPinConfigure(GPIO_PQ3_EPI0S23);
	GPIOPinTypeEPI(GPIO_PORTQ_BASE, GPIO_PIN_3);	
	MAP_GPIOPinConfigure(GPIO_PK7_EPI0S24);
	GPIOPinTypeEPI(GPIO_PORTK_BASE, GPIO_PIN_7);
	MAP_GPIOPinConfigure(GPIO_PK6_EPI0S25);
	GPIOPinTypeEPI(GPIO_PORTK_BASE, GPIO_PIN_6);	
	MAP_GPIOPinConfigure(GPIO_PB2_EPI0S27);
	GPIOPinTypeEPI(GPIO_PORTB_BASE, GPIO_PIN_2);*/
	#if 0
    //
    // EPI0S4 ~ EPI0S7: C4 ~ 7
    //
    ui32Val = HWREG(GPIO_PORTC_BASE + GPIO_O_PCTL);
    ui32Val &= 0x0000FFFF;
    ui32Val |= 0xFFFF0000;
    HWREG(GPIO_PORTC_BASE + GPIO_O_PCTL) = ui32Val;

    //
    // EPI0S8 ~ EPI0S9: A6 ~ 7
    //
    ui32Val = HWREG(GPIO_PORTA_BASE + GPIO_O_PCTL);
    ui32Val &= 0x00FFFFFF;
    ui32Val |= 0xFF000000;
    HWREG(GPIO_PORTA_BASE + GPIO_O_PCTL) = ui32Val;

    //
    // EPI0S10 ~ EPI0S11: G0 ~ 1
    //
    ui32Val = HWREG(GPIO_PORTG_BASE + GPIO_O_PCTL);
    ui32Val &= 0xFFFFFF00;
    ui32Val |= 0x000000FF;
    HWREG(GPIO_PORTG_BASE + GPIO_O_PCTL) = ui32Val;

    //
    // EPI0S12 ~ EPI0S15: M0 ~ 3
    //
    ui32Val = HWREG(GPIO_PORTM_BASE + GPIO_O_PCTL);
    ui32Val &= 0xFFFF0000;
    ui32Val |= 0x0000FFFF;
    HWREG(GPIO_PORTM_BASE + GPIO_O_PCTL) = ui32Val;

    //
    // EPI0S16 ~ EPI0S19: L0 1
    //
    ui32Val = HWREG(GPIO_PORTL_BASE + GPIO_O_PCTL);
    ui32Val &= 0xFFFFFF00;
    ui32Val |= 0x000000FF;
    HWREG(GPIO_PORTL_BASE + GPIO_O_PCTL) = ui32Val;
    //
    // EPI0S20 : Q0
    //
    //ui32Val = HWREG(GPIO_PORTQ_BASE + GPIO_O_PCTL);
    //ui32Val &= 0xFFFFFFF0;
    //ui32Val |= 0x0000000F;
    //HWREG(GPIO_PORTQ_BASE + GPIO_O_PCTL) = ui32Val;
    //
    // EPI0S26 : L4
    //
    //ui32Val = HWREG(GPIO_PORTL_BASE + GPIO_O_PCTL);
    //ui32Val &= 0xFFF0FFFF;
    //ui32Val |= 0x000F0000;
    //HWREG(GPIO_PORTL_BASE + GPIO_O_PCTL) = ui32Val;

    //
    // EPI0S28 : B3
    //
    ui32Val = HWREG(GPIO_PORTB_BASE + GPIO_O_PCTL);
    ui32Val &= 0xFFFF0FFF;
    ui32Val |= 0x0000F000;
    HWREG(GPIO_PORTB_BASE + GPIO_O_PCTL) = ui32Val;

    //
    // EPI0S29: N2 3
    //
    ui32Val = HWREG(GPIO_PORTN_BASE + GPIO_O_PCTL);
    ui32Val &= 0xFFFF00FF;
    ui32Val |= 0x0000FF00;
    HWREG(GPIO_PORTN_BASE + GPIO_O_PCTL) = ui32Val;

    //
    // EPI0S00 ~ EPI0S03: K0 ~ 3
    //
    ui32Val = HWREG(GPIO_PORTK_BASE + GPIO_O_PCTL);
    ui32Val &= 0xFFFF0000;
    ui32Val |= 0x0000FFFF;
    HWREG(GPIO_PORTK_BASE + GPIO_O_PCTL) = ui32Val;

    //
    // Configure the GPIO pins for EPI mode.  All the EPI pins require 8mA
    // drive strength in push-pull operation.  This step also gives control of
    // pins to the EPI module.
    //
    GPIOPinTypeEPI(GPIO_PORTA_BASE, EPI_PORTA_PINS);
    GPIOPinTypeEPI(GPIO_PORTB_BASE, EPI_PORTB_PINS);
    GPIOPinTypeEPI(GPIO_PORTC_BASE, EPI_PORTC_PINS);
    GPIOPinTypeEPI(GPIO_PORTG_BASE, EPI_PORTG_PINS);
    GPIOPinTypeEPI(GPIO_PORTK_BASE, EPI_PORTK_PINS);
    GPIOPinTypeEPI(GPIO_PORTL_BASE, EPI_PORTL_PINS);
    GPIOPinTypeEPI(GPIO_PORTM_BASE, EPI_PORTM_PINS);
    GPIOPinTypeEPI(GPIO_PORTN_BASE, EPI_PORTN_PINS);
	#endif
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
	EPIConfigHB8Set(EPI0_BASE, (EPI_HB8_MODE_SRAM| EPI_HB8_CSCFG_CS|EPI_HB8_IN_READY_EN/*| EPI_HB8_RDWAIT_3| EPI_HB8_WRWAIT_3*/),0);
//	EPIConfigHB8TimingSet(EPI0_BASE,0,EPI_HB8_RDWAIT_MINUS_ENABLE|EPI_HB8_WRWAIT_MINUS_ENABLE);
	//EPIConfigHB8CSSet(EPI0_BASE,1,(EPI_HB8_MODE_ADDEMUX |EPI_HB8_WRWAIT_3|EPI_HB8_RDWAIT_3 ));

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
               g_pui8EPISdram[SRAM_END_ADDRESS - 1]);
    rt_kprintf("     Mem[0x6000.03FF] = 0x%02x\n\n",
               g_pui8EPISdram[SRAM_END_ADDRESS]);

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
    g_pui8EPISdram[SRAM_END_ADDRESS - 1] = 0xdc;
    g_pui8EPISdram[SRAM_END_ADDRESS] = 0x2a;

    //
    // Read back the data you wrote, and display it on the console.
    //
    rt_kprintf("  SRAM Read:\n");
    rt_kprintf("     Mem[0x6000.0000] = 0x%02x\n",
               g_pui8EPISdram[SRAM_START_ADDRESS]);
    rt_kprintf("     Mem[0x6000.0001] = 0x%02x\n",
               g_pui8EPISdram[SRAM_START_ADDRESS + 1]);
    rt_kprintf("     Mem[0x6000.03FE] = 0x%02x\n",
               g_pui8EPISdram[SRAM_END_ADDRESS - 1]);
    rt_kprintf("     Mem[0x6000.03FF] = 0x%02x\n\n",
               g_pui8EPISdram[SRAM_END_ADDRESS]);
    //
    // Check the validity of the data.
    //
    if((g_pui8EPISdram[SRAM_START_ADDRESS] == 0xab) &&
      (g_pui8EPISdram[SRAM_START_ADDRESS + 1] == 0x12) &&
       (g_pui8EPISdram[SRAM_END_ADDRESS - 1] == 0xdc) &&
       (g_pui8EPISdram[SRAM_END_ADDRESS] == 0x2a))
    {
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
		for(i=768;i<1024;i++)
			g_pui8EPISdram[i]=i-768;
		for(i=0;i<1024;i++)
		{
			if(g_pui8EPISdram[i]!=(i%256))
				rt_kprintf("<1024> %d failed\r\n",i);
			rt_kprintf("0x%02x ",g_pui8EPISdram[i]);			
			if(((i+1)%16)==0)
				rt_kprintf("\n");
			if(((i+1)%256)==0)
				rt_kprintf("\n");
		}
		rt_kprintf("\n");
        return(0);
    }

	
    //
    // Display on the console that there was an error.
    //
    rt_kprintf("Read and/or write failure!");
    rt_kprintf(" Check if your SRAM card is plugged in.");

    //
    // Read and/or write operations were unsuccessful.  Wait in while(1) loop
    // for debugging.
    //
    while(1)
    {
    }
}

