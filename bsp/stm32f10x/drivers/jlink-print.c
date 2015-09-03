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
//
// STIM word and byte acces
#define ITM_STIM_U32  (*(volatile unsigned int*)0xE0000000)
#define ITM_STIM_U8   (*(volatile char*)0xE0000000)

// The stimulus port from which SWO data is received and displayed.
unsigned int ITM_PORT_BIT0 = 0;

// Has to be calculated according to the CPU speed and the output baud rate
unsigned int TargetDiv = 32;

void SWO_Enable( void )
{
  unsigned int StimulusRegs;
  //
  // Enable access to SWO registers
  //
  DEMCR |= ( 1 << 24 );
  ITM_LSR = 0xC5ACCE55;
  //
  // Initially disable ITM and stimulus port
  // To make sure that nothing is transferred via SWO
  // when changing the SWO prescaler etc.
  //
  StimulusRegs = ITM_ENA;
  StimulusRegs &= ~( 1 << ITM_PORT_BIT0 );
  ITM_ENA = StimulusRegs; // Disable ITM stimulus port
  ITM_TCR = 0;            // Disable ITM

  //
  // Initialize SWO (prescaler, etc.)
  //
  TPIU_SPPR = 0x00000002;     // Select NRZ mode
  TPIU_ACPR = TargetDiv - 1;  // Example: 72/48 = 1,5 MHz
  ITM_TPR = 0x00000000;
  DWT_CTRL = 0x400003FE;
  FFCR = 0x00000100;
  //
  // Enable ITM and stimulus port
  //
  ITM_TCR = 0x1000D; // Enable ITM
  ITM_ENA = StimulusRegs | ( 1 << ITM_PORT_BIT0 ); // Enable ITM stimulus port
}

// Prints a character to the ITM_STIM register in order to provide data for SWO
void SWO_PrintChar( char c )
{
  // Check if SWO is set up. If it is not,
  // return to avoid that a program hangs if no debugger is connected.
  //
  // Check if DEBUGEN in DHCSR is set
  //
  if ( ( DHCSR & 1 ) != 1 )
    return;

  //
  // Check if TRACENA in DEMCR is set
  //
  if ( ( DEMCR & ( 1 << 24 ) ) == 0 )
    return;

  //
  // Check if ITM_TRC is enabled
  //
  if ( ( ITM_TCR & ( 1 << 22 ) ) == 1 )
    return;

  //
  // Check if stimulus port 0 is enabled
  //
  if ( ( ITM_ENA & 1 ) == 0 )
    return;

  //
  // Wait until STIMx is ready to accept at least 1 word
  //
  while ( ( ITM_STIM_U8 & 1 ) == 0 )
  {

  }

  ITM_STIM_U8 = c;
}

// Prints a string via SWO
void SWO_PrintString( const char *s )
{
  //
  // Print out character per character
  //
  while ( *s )
  {
    SWO_PrintChar( *s++ );
  }
}
void rt_hw_console_output(const char *str)
{
	SWO_PrintString(str);
}
