#ifndef __DRV_TIMER_H
#define __DRV_TIMER_H

#include "rt_board.h"

/*-----------------------------------ErrorID-----------------------------------------*/

typedef    int	ErrorID; 
#define  Function_OK      0
#define  TimesIDError     1
#define  TimesCHIDError   2
#define  LineIDError      3
#define  WrongFrequency   4
#define  WrongPulseWidth  5
#define  PWMIDError       6
#define  PWMCHIDError     7

/*-----------------------------------Hardware Lines-----------------------------------------*/
/* TC-- Timer-PWM*/
#define TC0_TIOA0_PIO        (PIO_PA0_IDX)
#define TC0_TIOA0_MUX        (IOPORT_MODE_MUX_B)
#define TC0_TIOA0_FLAGS      (IOPORT_MODE_MUX_B)
#define TC0_TIOA0_MASK       PIO_PA0
#define TC0_TIOA0_ID         ID_PIOA
#define TC0_TIOA0_TYPE       PIO_PERIPH_B
#define TC0_TIOA0_ATTR       PIO_DEFAULT

#define TC0_TIOB0_PIO        (PIO_PA1_IDX)
#define TC0_TIOB0_MUX        (IOPORT_MODE_MUX_B)
#define TC0_TIOB0_FLAGS      (IOPORT_MODE_MUX_B)
#define TC0_TIOB0_MASK       PIO_PA1
#define TC0_TIOB0_ID         ID_PIOA
#define TC0_TIOB0_TYPE       PIO_PERIPH_B
#define TC0_TIOB0_ATTR       PIO_DEFAULT

#define TC2_TIOB7_PIO        (PIO_PC9_IDX)
#define TC2_TIOB7_MUX        (IOPORT_MODE_MUX_B)
#define TC2_TIOB7_FLAGS      (IOPORT_MODE_MUX_B)
#define TC2_TIOB7_MASK       PIO_PC9
#define TC2_TIOB7_ID         ID_PIOC
#define TC2_TIOB7_TYPE       PIO_PERIPH_B
#define TC2_TIOB7_ATTR       PIO_DEFAULT

#define TC2_TIOB8_PIO        (PIO_PC12_IDX)
#define TC2_TIOB8_MUX        (IOPORT_MODE_MUX_B)
#define TC2_TIOB8_FLAGS      (IOPORT_MODE_MUX_B)
#define TC2_TIOB8_MASK       PIO_PC12
#define TC2_TIOB8_ID         ID_PIOC
#define TC2_TIOB8_TYPE       PIO_PERIPH_B
#define TC2_TIOB8_ATTR       PIO_DEFAULT

/* PWM Generator*/
/** PWMC0 pin definitions. */
#define PWMC0_PWMH0_PIO      PIO_PA23_IDX
#define PWMC0_PWMH0_FLAGS    (IOPORT_MODE_MUX_B)
#define PWMC0_PWMH0_CHANNEL   PWM_CHANNEL_0

#define PWMC0_PWMH1_PIO      PIO_PA2_IDX
#define PWMC0_PWMH1_FLAGS    (IOPORT_MODE_MUX_A)
#define PWMC0_PWMH1_CHANNEL   PWM_CHANNEL_1

#define PWMC0_PWML2_PIO      PIO_PB13_IDX
#define PWMC0_PWML2_FLAGS    (IOPORT_MODE_MUX_A)
#define PWMC0_PWML2_CHANNEL   PWM_CHANNEL_2

#define PWMC0_PWMH3_PIO      PIO_PC13_IDX
#define PWMC0_PWMH3_FLAGS    (IOPORT_MODE_MUX_B)
#define PWMC0_PWMH3_CHANNEL   PWM_CHANNEL_3

/** PWMC1 pin definitions. */
#define PWMC1_PWMH0_PIO      PIO_PD1_IDX
#define PWMC1_PWMH0_FLAGS    (IOPORT_MODE_MUX_B)
#define PWMC1_PWMH0_CHANNEL   PWM_CHANNEL_0

#define PWMC1_PWMH1_PIO      PIO_PD3_IDX
#define PWMC1_PWMH1_FLAGS    (IOPORT_MODE_MUX_B)
#define PWMC1_PWMH1_CHANNEL   PWM_CHANNEL_1

#define PWMC1_PWMH2_PIO      PIO_PD5_IDX
#define PWMC1_PWMH2_FLAGS    (IOPORT_MODE_MUX_B)
#define PWMC1_PWMH2_CHANNEL   PWM_CHANNEL_2

#define PWMC1_PWMH3_PIO      PIO_PD7_IDX
#define PWMC1_PWMH3_FLAGS    (IOPORT_MODE_MUX_B)
#define PWMC1_PWMH3_CHANNEL   PWM_CHANNEL_3

/*-----------------------------------Function Definitions-----------------------------------------*/
extern ErrorID GeneratePWMByTimers(int TimerID,int Timer_CHID,int LineID,int Freq,int PulseWidth);
extern ErrorID ChangePulseWidthByTimers(int TimerID,int Timer_CHID,int LineID,int Freq , int PulseWidth);
extern bool StartTimesInterrupt(int TimerID,int Timer_CHID,int Freq,int priority,void *callback_function);
extern bool StopTimersInterrupt(int TimerID,int Timer_CHID);

extern ErrorID GeneratePWMByPWM(int PWMID,int PWM_CHID,int LineID,int Freq,int PulseWidth);
extern bool ChangePulseWidthByPWM(int PWMID,int PWM_CHID,int PulseWidth);

#endif

