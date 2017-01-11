#include <rtthread.h>
#include "board.h"

#include "drv_afec.h"
bool AFEC_init(int AFEC_ID, int CH_ID) 
{	
	Afec *afec_base;
	int id;

	if (AFEC_ID != 0 && AFEC_ID != 1)
		return false;

	if (AFEC_ID == 0 && CH_ID != 7 && CH_ID !=8)
		return false;

	if (AFEC_ID ==1 && CH_ID != 6)
		return false;

	if (AFEC_ID ==0)
	{
		afec_base = AFEC0;
		id = ID_AFEC0;		
	}
	else
	{
		afec_base = AFEC1;
		id = ID_AFEC1;
	}
	AFEC_Initialize( afec_base, id );	
	AFEC_SetModeReg(afec_base, 0
		| AFEC_EMR_RES_NO_AVERAGE			
		| (1 << AFEC_MR_TRANSFER_Pos)			
		| (2 << AFEC_MR_TRACKTIM_Pos)			
		| AFEC_MR_ONE			
		| AFEC_MR_SETTLING_AST3			
		| AFEC_MR_STARTUP_SUT64);	
	AFEC_SetClock(afec_base, 6000000, BOARD_MCK);	
	AFEC_SetExtModeReg(afec_base, 0
		| AFEC_EMR_RES_NO_AVERAGE			
		| AFEC_EMR_TAG			
		| AFEC_EMR_STM );
	afec_base->AFEC_ACR = AFEC_ACR_IBCTL(2)
		| (1 << 4)
		| AFEC_ACR_PGA0_ON
		| AFEC_ACR_PGA1_ON;
	if (CH_ID == 6)
	{
		AFEC_SetChannelGain(afec_base, AFEC_CGR_GAIN6(0));
	}
	else if (CH_ID == 7)
	{
		AFEC_SetChannelGain(afec_base, AFEC_CGR_GAIN7(0));
	}
	else
	{
		AFEC_SetChannelGain(afec_base, AFEC_CGR_GAIN8(0));
	}
	AFEC_SetAnalogOffset(afec_base, CH_ID, 0x200);
	AFEC_SetTrigger(afec_base, AFEC_MR_TRGSEL(AFEC_MR_TRGSEL_AFEC_TRIG0));
	AFEC_SetCompareMode(afec_base, AFEC_EMR_CMPMODE(AFEC_EMR_CMPMODE_IN)|
		AFEC_EMR_CMPSEL(CH_ID)|AFEC_EMR_CMPFILTER(0));
	AFEC_SetComparisonWindow(afec_base, AFEC_CWR_LOWTHRES(0)|AFEC_CWR_HIGHTHRES(0xfff));
	
	AFEC_EnableChannel(afec_base, CH_ID);	

	rt_kprintf("AFEC%d->AFEC_ACR 0x%x, MR 0x%x, EMR 0x%x, CR 0x%x\n", AFEC_ID,
		afec_base->AFEC_ACR, afec_base->AFEC_MR,
		afec_base->AFEC_EMR, afec_base->AFEC_CR);
	return true;
}

bool AFEC_get_data(int AFEC_ID, int CH_ID, uint32_t *data)
{
	Afec *afec_base;
	int offset;
	if (AFEC_ID != 0 && AFEC_ID != 1)
		return false;

	if (AFEC_ID == 0 && CH_ID != 7 && CH_ID !=8)
		return false;

	if (AFEC_ID ==1 && CH_ID != 6)
		return false;

	if (AFEC_ID ==0)
	{
		afec_base = AFEC0;
	}
	else
	{
		afec_base = AFEC1;
	}

	if (CH_ID == 6)
	{
		offset = AFEC_IER_EOC6;
		AFEC_SetChannelGain(afec_base, AFEC_CGR_GAIN6(0));
	}
	else if (CH_ID == 7)
	{
		offset = AFEC_IER_EOC7;
		AFEC_SetChannelGain(afec_base, AFEC_CGR_GAIN7(0));
	}
	else
	{
		offset = AFEC_IER_EOC8;
		AFEC_SetChannelGain(afec_base, AFEC_CGR_GAIN8(0));
	}
	AFEC_SetAnalogOffset(afec_base, CH_ID, 0x200);
	AFEC_SetCompareMode(afec_base, AFEC_EMR_CMPMODE(AFEC_EMR_CMPMODE_IN)|
						AFEC_EMR_CMPSEL(CH_ID)|AFEC_EMR_CMPFILTER(0));
	AFEC_EnableChannel(afec_base, CH_ID);	
	afec_base->AFEC_IER |= offset;
	afec_base->AFEC_CSELR = CH_ID;
	afec_base->AFEC_CR = AFEC_CR_START;
	while(!(AFEC_GetStatus(afec_base)&offset))
		rt_thread_delay(1);
	*data = afec_base->AFEC_CDR;
	rt_kprintf("data %d\n", *data);
	return true;
}
#ifdef RT_USING_FINSH
#include <finsh.h>
static void afec_init_t(int AFEC_ID, int CH_ID)
{
	int result = AFEC_init(AFEC_ID,CH_ID);
	if (result == true)
		rt_kprintf("AFEC_init ok\n");
	else		
		rt_kprintf("AFEC_init failed\n");
}
static void afec_get(int AFEC_ID, int CH_ID)	
{
	uint32_t data;
	int result = AFEC_get_data(AFEC_ID,CH_ID, &data);
	if (result == true)
		rt_kprintf("AFEC_get_data %d\n", data);
	else		
		rt_kprintf("AFEC_get_data failed\n");
}
FINSH_FUNCTION_EXPORT(afec_init_t, test AFEC_init);
FINSH_FUNCTION_EXPORT(afec_get, test AFEC_get_data);
#endif
