#include "stm32f10x.h"
#include "board.h"
#include <rtdevice.h>
#define ITM_ENA   (*(volatile unsigned int*)0xE0000E00) // ITM Enable
#define ITM_TPR   (*(volatile unsigned int*)0xE0000E40) // Trace Privilege Register
#define ITM_TCR   (*(volatile unsigned int*)0xE0000E80) // ITM Trace Control Reg.
#define ITM_LSR   (*(volatile unsigned int*)0xE0000FB0) // ITM Lock Status Register
#define DHCSR     (*(volatile unsigned int*)0xE000EDF0) // Debug register
#define DEMCR     (*(volatile unsigned int*)0xE000EDFC) // Debug register
#define TPIU_ACPR (*(volatile unsigned int*)0xE0040010) // Async Clock presacler register
#define TPIU_SPPR (*(volatile unsigned int*)0xE00400F0) // Selected Pin Protocol Register
#define DWT_CTRL  (*(volatile unsigned int*)0xE0001000) // DWT Control Register
#define FFCR      (*(volatile unsigned int*)0xE0040304) // Formatter and flush Control Register

#define ITM_STIM_U32  (*(volatile unsigned int*)0xE0000000)
#define ITM_STIM_U8   (*(volatile char*)0xE0000000)

unsigned int ITM_PORT_BIT0 = 0;

unsigned int TargetDiv = 32;

void SWO_Enable( void )
{
	unsigned int StimulusRegs;
	DEMCR |= ( 1 << 24 );
	ITM_LSR = 0xC5ACCE55;
	StimulusRegs = ITM_ENA;
	StimulusRegs &= ~( 1 << ITM_PORT_BIT0 );
	ITM_ENA = StimulusRegs; 
	ITM_TCR = 0;            

	TPIU_SPPR = 0x00000002;     
	TPIU_ACPR = TargetDiv - 1;
	ITM_TPR = 0x00000000;
	DWT_CTRL = 0x400003FE;
	FFCR = 0x00000100;
	ITM_TCR = 0x1000D;
	ITM_ENA = StimulusRegs | ( 1 << ITM_PORT_BIT0 );
}

void SWO_PrintChar( char c )
{
	if ( ( DHCSR & 1 ) != 1 )
		return;

	if ( ( DEMCR & ( 1 << 24 ) ) == 0 )
		return;

	if ( ( ITM_TCR & ( 1 << 22 ) ) == 1 )
		return;

	if ( ( ITM_ENA & 1 ) == 0 )
		return;

	while ( ( ITM_STIM_U8 & 1 ) == 0 )
	{

	}

	ITM_STIM_U8 = c;
}
void rt_hw_console_output(const char *str)
{
	while ( *str )
      SWO_PrintChar( *str++ );
}
