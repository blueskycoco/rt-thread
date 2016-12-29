#include <rtthread.h>
#include "board.h"

#include "drv_timer.h"
#define PIN_TC0_CH0   {PIO_PA0B_TIOA0, PIOA, ID_PIOA, PIO_PERIPH_B, PIO_DEFAULT}
#define PIN_TC0_CH1   {PIO_PA1B_TIOB0, PIOA, ID_PIOA, PIO_PERIPH_B, PIO_DEFAULT}
#define PIN_TC2_CH1   {PIO_PC9B_TIOB7, PIOC, ID_PIOC, PIO_PERIPH_B, PIO_DEFAULT}
#define PIN_TC2_CH2   {PIO_PC12B_TIOB8, PIOC, ID_PIOC, PIO_PERIPH_B, PIO_DEFAULT}
static const Pin pPwmpins[] = {PIN_TC0_CH0, PIN_TC0_CH1, PIN_TC2_CH1, PIN_TC2_CH2};
const uint32_t divisors[5] = {2, 8, 32, 128, BOARD_MCK / 32768};

ErrorID GeneratePWMByTimers (int TimerID,int Timer_CHID,int LineID,int Freq,int PulseWidth)
{
	Tc *tc_base;
	uint32_t ra, rc;
	double temp;
	uint32_t clockSelection;
	
	if (TimerID != 0 && TimerID !=1 
		&& TimerID !=2 && TimerID != 3)
		return TimesIDError;

	if (Timer_CHID != 0 && Timer_CHID != 1
		&& Timer_CHID != 2)
		return TimesCHIDError;

	if (LineID != 1 && LineID !=2)
		return LineIDError;
	
	if (Freq < 1 || Freq > 1000)
		return WrongFrequency;

	if (PulseWidth < 10 || PulseWidth > 5000)
		return WrongPulseWidth;

	if (Freq == 1000 && PulseWidth >= 1000)
		return WrongPulseWidth;

	if (TimerID == 3)
		tc_base = TC3;
	else
		tc_base = (Tc *)(0x4000C000 + TimerID*0x4000);
	
	PIO_Configure( pPwmpins, PIO_LISTSIZE(pPwmpins));

	if (TimerID == 0)		
		PMC_EnablePeripheral(ID_TC0);
	else if (TimerID == 2) {
		if (Timer_CHID == 1)
			PMC_EnablePeripheral(ID_TC7);
		else if (Timer_CHID == 2)
			PMC_EnablePeripheral(ID_TC8);
	}

	if (Freq > 290)
		clockSelection = TC_CMR_TCCLKS_TIMER_CLOCK2;
	else if (Freq > 72)
		clockSelection = TC_CMR_TCCLKS_TIMER_CLOCK3;
	else
		clockSelection = TC_CMR_TCCLKS_TIMER_CLOCK4;
	
	rc = (BOARD_MCK / divisors[clockSelection])/Freq;
	temp = (BOARD_MCK / divisors[clockSelection])/1000000.0;
	ra = rc - temp*PulseWidth;
	tc_base->TC_CHANNEL[Timer_CHID].TC_CCR = TC_CCR_CLKDIS;	
	tc_base->TC_CHANNEL[Timer_CHID].TC_IDR = 0xFFFFFFFF;	
	tc_base->TC_CHANNEL[Timer_CHID].TC_SR;	
	tc_base->TC_CHANNEL[Timer_CHID].TC_RC = rc;

	if (LineID == 1) {
		tc_base->TC_CHANNEL[Timer_CHID].TC_CMR =		
		clockSelection									 
		|TC_CMR_EEVT_XC0
		|TC_CMR_WAVSEL_UP_RC
		|TC_CMR_WAVE
		|TC_CMR_ACPA_SET
		|TC_CMR_ACPC_CLEAR;
		tc_base->TC_CHANNEL[Timer_CHID].TC_RA = ra;
	} else {
		tc_base->TC_CHANNEL[Timer_CHID].TC_CMR =		
		clockSelection									 
		|TC_CMR_EEVT_XC0
		|TC_CMR_WAVSEL_UP_RC
		|TC_CMR_WAVE
		|TC_CMR_BCPB_SET
		|TC_CMR_BCPC_CLEAR;
		tc_base->TC_CHANNEL[Timer_CHID].TC_RB = ra;
	}

	tc_base->TC_CHANNEL[Timer_CHID].TC_CCR =  TC_CCR_CLKEN | TC_CCR_SWTRG;	
	rt_kprintf ("Start waveform: Frequency = %d Hz,pulseWidth = %2dus,TC_CMR %x TC_RA %x TC_RB %x TC_RC %x\n\r",			
		Freq,			
		PulseWidth,
		tc_base->TC_CHANNEL[Timer_CHID].TC_CMR,
		tc_base->TC_CHANNEL[Timer_CHID].TC_RA,
		tc_base->TC_CHANNEL[Timer_CHID].TC_RB,
		tc_base->TC_CHANNEL[Timer_CHID].TC_RC);
    return Function_OK;
}
ErrorID ChangePulseWidthByTimers(int TimerID,int Timer_CHID,int LineID,int Freq , int PulseWidth)
{
	Tc *tc_base;
	uint32_t ra, rc;
	double temp;
	uint32_t clockSelection;
	
	if (TimerID != 0 && TimerID !=1 
		&& TimerID !=2 && TimerID != 3)
		return TimesIDError;

	if (Timer_CHID != 0 && Timer_CHID != 1
		&& Timer_CHID != 2)
		return TimesCHIDError;

	if (LineID != 1 && LineID !=2)
		return LineIDError;
	
	if (Freq < 1 || Freq > 1000)
		return WrongFrequency;

	if (PulseWidth < 10 || PulseWidth > 5000)
		return WrongPulseWidth;

	if (Freq == 1000 && PulseWidth >= 1000)
		return WrongPulseWidth;

	if (TimerID == 3)
		tc_base = TC3;
	else
		tc_base = (Tc *)(0x4000C000 + TimerID*0x4000);
	
	if (Freq > 290)
		clockSelection = TC_CMR_TCCLKS_TIMER_CLOCK2;
	else if (Freq > 72)
		clockSelection = TC_CMR_TCCLKS_TIMER_CLOCK3;
	else
		clockSelection = TC_CMR_TCCLKS_TIMER_CLOCK4;
	
	rc = (BOARD_MCK / divisors[clockSelection])/Freq;	
	temp = (BOARD_MCK / divisors[clockSelection])/1000000.0;
	ra = rc - temp*PulseWidth;
	tc_base->TC_CHANNEL[Timer_CHID].TC_RC = rc;

	if (LineID == 1) {
		tc_base->TC_CHANNEL[Timer_CHID].TC_RA = ra;
	} else {
		tc_base->TC_CHANNEL[Timer_CHID].TC_RB = ra;
	}
	rt_kprintf ("Change waveform: Frequency = %d Hz,pulseWidth = %2dus,TC_CMR %x TC_RA %x TC_RB %x TC_RC %x\n\r",			
		Freq,			
		PulseWidth,
		tc_base->TC_CHANNEL[Timer_CHID].TC_CMR,
		tc_base->TC_CHANNEL[Timer_CHID].TC_RA,
		tc_base->TC_CHANNEL[Timer_CHID].TC_RB,
		tc_base->TC_CHANNEL[Timer_CHID].TC_RC);
	return Function_OK;
}

#ifdef RT_USING_FINSH
#include <finsh.h>
static void pwm_timerG(int TimerID,int Timer_CHID,int LineID,int Freq,int PulseWidth)
{
	int result = GeneratePWMByTimers(TimerID,Timer_CHID,LineID,Freq,PulseWidth);
	if (result == 0)
		rt_kprintf("GeneratePWMByTimers Function_OK\n");
	else if (result == 1)
		rt_kprintf("GeneratePWMByTimers TimesIDError\n");	
	else if (result == 2)
		rt_kprintf("GeneratePWMByTimers TimesCHIDError\n");
	else if (result == 3)
		rt_kprintf("GeneratePWMByTimers LineIDError\n");
	else if (result == 4)
		rt_kprintf("GeneratePWMByTimers WrongFrequency\n");
	else if (result == 5)
		rt_kprintf("GeneratePWMByTimers WrongPulseWidth\n");
}
static void pwm_timerC(int TimerID,int Timer_CHID,int LineID,int Freq,int PulseWidth)
{
	int result = ChangePulseWidthByTimers(TimerID,Timer_CHID,LineID,Freq,PulseWidth);
	if (result == 0)
		rt_kprintf("ChangePulseWidthByTimers Function_OK\n");
	else if (result == 1)
		rt_kprintf("ChangePulseWidthByTimers TimesIDError\n");	
	else if (result == 2)
		rt_kprintf("ChangePulseWidthByTimers TimesCHIDError\n");
	else if (result == 3)
		rt_kprintf("ChangePulseWidthByTimers LineIDError\n");
	else if (result == 4)
		rt_kprintf("ChangePulseWidthByTimers WrongFrequency\n");
	else if (result == 5)
		rt_kprintf("ChangePulseWidthByTimers WrongPulseWidth\n");
}

FINSH_FUNCTION_EXPORT(pwm_timerG, test pwm timer GeneratePWMByTimers);
FINSH_FUNCTION_EXPORT(pwm_timerC, test pwm timer ChangePulseWidthByTimers);
#endif
