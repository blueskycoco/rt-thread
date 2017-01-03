#include <rtthread.h>
#include "board.h"

#include "drv_timer.h"
#define PIN_TC0_CH0   {PIO_PA0B_TIOA0, PIOA, ID_PIOA, PIO_PERIPH_B, PIO_DEFAULT}
#define PIN_TC0_CH1   {PIO_PA1B_TIOB0, PIOA, ID_PIOA, PIO_PERIPH_B, PIO_DEFAULT}
#define PIN_TC2_CH1   {PIO_PC9B_TIOB7, PIOC, ID_PIOC, PIO_PERIPH_B, PIO_DEFAULT}
#define PIN_TC2_CH2   {PIO_PC12B_TIOB8, PIOC, ID_PIOC, PIO_PERIPH_B, PIO_DEFAULT}
#define PIN_PWM0_CH0  {PIO_PA23B_PWMC0_PWMH0, PIOA, ID_PIOA, PIO_PERIPH_B, PIO_DEFAULT}
#define PIN_PWM0_CH1  {PIO_PA2A_PWMC0_PWMH1, PIOA, ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}
#define PIN_PWM0_CH2  {PIO_PB13A_PWMC0_PWML2, PIOB, ID_PIOB, PIO_PERIPH_A, PIO_DEFAULT}
#define PIN_PWM0_CH3  {PIO_PC13B_PWMC0_PWMH3, PIOC, ID_PIOC, PIO_PERIPH_B, PIO_DEFAULT}
#define PIN_PWM1_CH0  {PIO_PD1B_PWMC1_PWMH0, PIOD, ID_PIOD, PIO_PERIPH_B, PIO_DEFAULT}
#define PIN_PWM1_CH1  {PIO_PD3B_PWMC1_PWMH1, PIOD, ID_PIOD, PIO_PERIPH_B, PIO_DEFAULT}
#define PIN_PWM1_CH2  {PIO_PD5B_PWMC1_PWMH2, PIOD, ID_PIOD, PIO_PERIPH_B, PIO_DEFAULT}
#define PIN_PWM1_CH3  {PIO_PD7B_PWMC1_PWMH3, PIOD, ID_PIOD, PIO_PERIPH_B, PIO_DEFAULT}
#define PIN_PWM_CTL   {PIO_PA29, PIOA, ID_PIOA, PIO_OUTPUT_1, PIO_DEFAULT}

static const Pin pPwmpins[] = {PIN_TC0_CH0, PIN_TC0_CH1, PIN_TC2_CH1, PIN_TC2_CH2};
static const Pin pPwmpins1[] = {PIN_PWM0_CH0, PIN_PWM0_CH1, PIN_PWM0_CH2, PIN_PWM0_CH3,
								PIN_PWM1_CH0,PIN_PWM1_CH1,PIN_PWM1_CH2,PIN_PWM1_CH3};
static const Pin pPwmCtl[] = {PIN_PWM_CTL};

const uint32_t divisors[5] = {2, 8, 32, 128, BOARD_MCK / 32768};
callback_t tc_callback;
int channel = 0;

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
ErrorID GeneratePWMByPWM(int PWMID,int PWM_CHID,int LineID,int Freq,int PulseWidth)
{	
	Pwm *pwm_base;
	uint16_t period,duty;
	int index;
	int polarity = 0;
	
	if (PWMID != 0 && PWMID !=1)
		return PWMIDError;

	if (PWM_CHID != 0 && PWM_CHID != 1
		&& PWM_CHID != 2 && PWM_CHID != 3)
		return PWMCHIDError;

	if (LineID != 1 && LineID !=2)
		return LineIDError;

	if (Freq < 1 || Freq > 1000)
		return WrongFrequency;

	if (PulseWidth < 10 || PulseWidth > 5000)
		return WrongPulseWidth;

	if (Freq == 1000 && PulseWidth >= 1000)
		return WrongPulseWidth;

	if (PWMID == 0)
		pwm_base = PWM0;
	else
		pwm_base = PWM1;
	
	PIO_Configure( pPwmpins1, PIO_LISTSIZE(pPwmpins1));
	
	if (PWMID == 0)		
		PMC_EnablePeripheral(ID_PWM0);
	else
		PMC_EnablePeripheral(ID_PWM1);
	if (Freq > 573)
		index = 4;
	else if (Freq > 290)
		index = 8;
	else if (Freq > 144)
		index = 16;
	else if (Freq > 72)
		index = 32;
	else if (Freq > 36)
		index = 64;
	else
		index = 128;

	if (PWMID == 0 && PWM_CHID == 2)
		polarity = PWM_CMR_CPOL;
	
	PWMC_ConfigureClocks(pwm_base, BOARD_MCK/index, BOARD_MCK/index, BOARD_MCK);	

	if (LineID == 1)
	{	
		PWMC_DisableChannel(pwm_base, PWM_CHID);
		PWMC_ConfigureChannelExt( pwm_base,
				PWM_CHID,
				PWM_CMR_CPRE_CLKA,
				0,
				polarity,
				0,0,0,0
				);
	}
	else
	{
		PWMC_DisableChannel(pwm_base, PWM_CHID);			
		PWMC_ConfigureChannelExt( pwm_base,
			PWM_CHID,
			PWM_CMR_CPRE_CLKB,
			0,
			polarity,
			0,0,0,0
			);
	}
	period = BOARD_MCK / (Freq * index);
	duty = period  - (double)(BOARD_MCK/(index*1000000.0))*PulseWidth;
	rt_kprintf("period %d, duty %d\n",period,duty);
	PWMC_SetPeriod(pwm_base, PWM_CHID, period);
	PWMC_SetDutyCycle(pwm_base, PWM_CHID, duty);
	PWMC_EnableChannel(pwm_base, PWM_CHID);

	return Function_OK;
}

ErrorID ChangePulseWidthByPWM(int PWMID,int PWM_CHID,int LineID,int Freq,int PulseWidth)
{	
	Pwm *pwm_base;
	uint16_t period,duty;
	int index;
	
	if (PWMID != 0 && PWMID !=1)
		return PWMIDError;

	if (PWM_CHID != 0 && PWM_CHID != 1
		&& PWM_CHID != 2 && PWM_CHID != 3)
		return PWMCHIDError;

	if (LineID != 1 && LineID !=2)
		return LineIDError;

	if (Freq < 1 || Freq > 1000)
		return WrongFrequency;

	if (PulseWidth < 10 || PulseWidth > 5000)
		return WrongPulseWidth;

	if (Freq == 1000 && PulseWidth >= 1000)
		return WrongPulseWidth;

	if (PWMID == 0)
		pwm_base = PWM0;
	else
		pwm_base = PWM1;
	
	if (Freq > 573)
		index = 4;
	else if (Freq > 290)
		index = 8;
	else if (Freq > 144)
		index = 16;
	else if (Freq > 72)
		index = 32;
	else if (Freq > 36)
		index = 64;
	else
		index = 128;
	
	period = BOARD_MCK / (Freq * index);
	duty = period  - (double)(BOARD_MCK/(index*1000000.0))*PulseWidth;
	rt_kprintf("period %d, duty %d\n",period,duty);
 	PWMC_SetDutyCycle(pwm_base, PWM_CHID, duty);
	//MC_EnableChannel(pwm_base, PWM_CHID);

	return Function_OK;
}
void PWM_enable(void)
{
	PIO_Configure(&pPwmCtl[0], 1);
	PIO_Clear(&pPwmCtl[0]);
}

void PWM_disable(void)
{
	PIO_Configure(&pPwmCtl[0], 1);
	PIO_Set(&pPwmCtl[0]);
}
void TC0_Handler(void)
{
	uint32_t status;	
	status = TC0->TC_CHANNEL[channel].TC_SR; 
	rt_kprintf("TC0_Handler %x \n", status);
	if ((status & TC_SR_CPCS) == TC_SR_CPCS) 
		tc_callback();
}
void TC1_Handler(void)
{
	uint32_t status;	
	status = TC1->TC_CHANNEL[channel].TC_SR; 
	rt_kprintf("TC1_Handler %x \n", status);
	if ((status & TC_SR_CPCS) == TC_SR_CPCS) 
		tc_callback();
}
void TC2_Handler(void)
{
	uint32_t status;	
	status = TC2->TC_CHANNEL[channel].TC_SR; 
	rt_kprintf("TC2_Handler %x \n", status);
	if ((status & TC_SR_CPCS) == TC_SR_CPCS) 
		tc_callback();
}
void TC3_Handler(void)
{
	uint32_t status;	
	status = TC3->TC_CHANNEL[channel].TC_SR; 
	rt_kprintf("TC3_Handler %x \n", status);
	if ((status & TC_SR_CPCS) == TC_SR_CPCS) 
		tc_callback();
}

bool StartTimesInterrupt(int TimerID,int Timer_CHID,int Freq,int priority,callback_t callback_function)
{
	Tc *tc_base;
	uint32_t rc;
	int irq;
	uint32_t clockSelection;
	
	if (TimerID != 0 && TimerID !=1 
		&& TimerID !=2 && TimerID != 3)
		return false;

	if (Timer_CHID != 0 && Timer_CHID != 1
		&& Timer_CHID != 2)
		return false;

	if (TimerID == 3)
		tc_base = TC3;
	else
		tc_base = (Tc *)(0x4000C000 + TimerID*0x4000);
	
	if (TimerID == 0)		
	{
		if (Timer_CHID == 1)
			PMC_EnablePeripheral(ID_TC1);
		else if (Timer_CHID == 2)
			PMC_EnablePeripheral(ID_TC2);
		else
			return false;
		irq = TC0_IRQn;
	}
	else if (TimerID == 1)
	{
		if (Timer_CHID == 0)
			PMC_EnablePeripheral(ID_TC3);
		else if (Timer_CHID == 1)
			PMC_EnablePeripheral(ID_TC4);
		else if (Timer_CHID == 2)
			PMC_EnablePeripheral(ID_TC5);
		else
			return false;
		irq = TC1_IRQn;
	}
	else if (TimerID == 2) 
	{
		if (Timer_CHID == 0)
			PMC_EnablePeripheral(ID_TC6);
		else
			return false;
		irq = TC2_IRQn;
	}
	else
	{
		if (Timer_CHID == 0)
			PMC_EnablePeripheral(ID_TC9);
		else if (Timer_CHID == 1)
			PMC_EnablePeripheral(ID_TC10);
		else if (Timer_CHID == 2)
			PMC_EnablePeripheral(ID_TC11);
		else
			return false;
		irq = TC3_IRQn;
	}
	tc_callback = callback_function;
	channel = Timer_CHID;
#if 0
	if (Freq > 290)
		clockSelection = TC_CMR_TCCLKS_TIMER_CLOCK2;
	else if (Freq > 72)
		clockSelection = TC_CMR_TCCLKS_TIMER_CLOCK3;
	else 
		clockSelection = TC_CMR_TCCLKS_TIMER_CLOCK4;
	
	rc = (BOARD_MCK / divisors[clockSelection])/Freq;
	tc_base->TC_CHANNEL[Timer_CHID].TC_CCR = TC_CCR_CLKDIS;	
	tc_base->TC_CHANNEL[Timer_CHID].TC_IDR = 0xFFFFFFFF;	
	tc_base->TC_CHANNEL[Timer_CHID].TC_SR;	
	tc_base->TC_CHANNEL[Timer_CHID].TC_RC = rc;

	tc_base->TC_CHANNEL[Timer_CHID].TC_CMR = clockSelection	| TC_CMR_CPCTRG;
	NVIC_DisableIRQ(irq);
	NVIC_ClearPendingIRQ(irq);
	NVIC_SetPriority(irq, priority);
	NVIC_EnableIRQ(irq);	
	tc_base->TC_CHANNEL[Timer_CHID].TC_IER = TC_IER_CPCS;
	tc_base->TC_CHANNEL[Timer_CHID].TC_CCR =  TC_CCR_CLKEN | TC_CCR_SWTRG;
#else
	uint32_t div;
	uint32_t tcclks;
	/** Configure TC for a 4Hz frequency and trigger on RC compare. */
	TC_FindMckDivisor(Freq, BOARD_MCK, &div, &tcclks, BOARD_MCK);

	TC_Configure(tc_base, Timer_CHID, tcclks | TC_CMR_CPCTRG);
	tc_base->TC_CHANNEL[Timer_CHID].TC_RC = (BOARD_MCK / div) / Freq;

	/* Configure and enable interrupt on RC compare */
	NVIC_ClearPendingIRQ(irq);
	NVIC_EnableIRQ(irq);

	tc_base->TC_CHANNEL[Timer_CHID].TC_IER = TC_IER_CPCS;

	/** Start the counter */
	TC_Start(tc_base, Timer_CHID);
#endif
	rt_kprintf ("StartTimesInterrupt: Frequency = %d Hz,TC_IER %x TC_CMR %x TC_RC %x\n\r",			
		Freq,
		tc_base->TC_CHANNEL[Timer_CHID].TC_IMR,
		tc_base->TC_CHANNEL[Timer_CHID].TC_CMR,
		tc_base->TC_CHANNEL[Timer_CHID].TC_RC);
    return true;

}
bool StopTimersInterrupt(int TimerID,int Timer_CHID)
{
	return true;
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
static void pwm_pwmG(int PWMID,int PWM_CHID,int LineID,int Freq,int PulseWidth)
{
	int result = GeneratePWMByPWM(PWMID,PWM_CHID,LineID,Freq,PulseWidth);
	if (result == 0)
		rt_kprintf("GeneratePWMByPWM Function_OK\n");
	else if (result == 1)
		rt_kprintf("GeneratePWMByPWM TimesIDError\n");	
	else if (result == 2)
		rt_kprintf("GeneratePWMByPWM TimesCHIDError\n");
	else if (result == 3)
		rt_kprintf("GeneratePWMByPWM LineIDError\n");
	else if (result == 4)
		rt_kprintf("GeneratePWMByPWM WrongFrequency\n");
	else if (result == 5)
		rt_kprintf("GeneratePWMByPWM WrongPulseWidth\n");
}
static void pwm_pwmC(int PWMID,int PWM_CHID,int LineID,int Freq,int PulseWidth)
{
	int result = ChangePulseWidthByPWM(PWMID,PWM_CHID,LineID,Freq,PulseWidth);
	if (result == 0)
		rt_kprintf("ChangePulseWidthByPWM Function_OK\n");
	else if (result == 1)
		rt_kprintf("ChangePulseWidthByPWM TimesIDError\n");	
	else if (result == 2)
		rt_kprintf("ChangePulseWidthByPWM TimesCHIDError\n");
	else if (result == 3)
		rt_kprintf("ChangePulseWidthByPWM LineIDError\n");
	else if (result == 4)
		rt_kprintf("ChangePulseWidthByPWM WrongFrequency\n");
	else if (result == 5)
		rt_kprintf("ChangePulseWidthByPWM WrongPulseWidth\n");
}
static void pwm_on()
{
	PWM_enable();
}
static void pwm_off()
{
	PWM_disable();
}
void tc_cb()
{
	rt_kprintf("in tc_cb\n");
}
static void timer_start(int TimerID,int Timer_CHID,int Freq,int priority)
{
	if(StartTimesInterrupt(TimerID,Timer_CHID,Freq,priority,(callback_t) tc_cb))
		rt_kprintf("StartTimesInterrupt ok\n");
	else
		rt_kprintf("StartTimesInterrupt failed\n");
}
FINSH_FUNCTION_EXPORT(pwm_timerG, test pwm timer GeneratePWMByTimers);
FINSH_FUNCTION_EXPORT(pwm_timerC, test pwm timer ChangePulseWidthByTimers);
FINSH_FUNCTION_EXPORT(pwm_pwmG, test pwm pwm GeneratePWMByPWM);
FINSH_FUNCTION_EXPORT(pwm_pwmC, test pwm timer ChangePulseWidthByPWM);
FINSH_FUNCTION_EXPORT(pwm_on, test pwm on);
FINSH_FUNCTION_EXPORT(pwm_off, test pwm off);
FINSH_FUNCTION_EXPORT(timer_start, test StartTimesInterrupt);

#endif
