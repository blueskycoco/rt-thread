#include <rtthread.h>
#include "board.h"

#include "drv_afec.h"
/** Global DMA driver for all transfer */
extern sXdmad dmaDrv;
/** Global AFE DMA instance */
static AfeDma Afed;
/** AFE command instance */
static AfeCmd AfeCommand;
static struct rt_semaphore afe_sem;
bool init = false;
static void _afe_Callback(int dummy, void* pArg)
{	
	rt_sem_release(&afe_sem);
}
static void _afe_dmaTransfer(Afec *base,int id, uint32_t *data, int size)
{
	AfeCommand.RxSize= size;
	AfeCommand.pRxBuff = data;
	AfeCommand.callback = (AfeCallback)_afe_Callback;
	Afe_ConfigureDma(&Afed, base, id, &dmaDrv);
	Afe_SendData(&Afed, &AfeCommand);
}
static void _afe_initialization(Afec *base,int id,int *channel,int num_ch) {
	if (!init)
	{
		rt_sem_init(&afe_sem, "afe_sem", 0, 0);
		init = true;
	}
	AFEC_Initialize(base, id);
	AFEC_SetModeReg(base,0
			| AFEC_MR_FREERUN_ON
			| AFEC_EMR_RES_NO_AVERAGE
			| (1 << AFEC_MR_TRANSFER_Pos)
			| (2 << AFEC_MR_TRACKTIM_Pos)
			| AFEC_MR_ONE
			| AFEC_MR_SETTLING_AST3
			| AFEC_MR_STARTUP_SUT64);

	AFEC_SetClock( base, 6000000, BOARD_MCK);
	AFEC_SetExtModeReg(base,
			0
			| AFEC_EMR_RES_NO_AVERAGE
			| AFEC_EMR_TAG
			| AFEC_EMR_STM);
	AFEC_SetAnalogControl(base, AFEC_ACR_IBCTL(1) | AFEC_ACR_PGA0_ON |
			AFEC_ACR_PGA1_ON);
	for (int i=0; i < num_ch; i++) {
		AFEC_SetAnalogOffset(base, channel[i], 0x200);
		AFEC_EnableChannel(base, channel[i]);
	}
}
bool AFEC_get_data(int AFEC_ID, uint32_t *data, int len)
{
	Afec *afec_base;
	int afeId = 0;
	int i;
	int channel[2] = {0};
	int num_ch=0;
	if (AFEC_ID != 0 && AFEC_ID != 1)
		return false;

	//if (AFEC_ID == 0 && CH_ID != 7 && CH_ID !=8)
	//	return false;

	//if (AFEC_ID ==1 && CH_ID != 6)
	//	return false;

	if (AFEC_ID ==0)
	{
		afec_base = AFEC0;
		afeId = ID_AFEC0;
		channel[0]=8;channel[1]=7;
		num_ch=2;
	}
	else
	{
		afec_base = AFEC1;
		afeId = ID_AFEC1;
		channel[0]=6;
		num_ch=1;
	}
	_afe_initialization(afec_base,afeId,channel,num_ch);
	_afe_dmaTransfer(afec_base,afeId,data,len);
	if(RT_ETIMEOUT == rt_sem_take(&afe_sem, RT_WAITING_FOREVER))
		rt_kprintf("afe dma timeout\n");
	return true;
}
void afec_get(int AFEC_ID,int len)	
{
	int i = 0;
	uint32_t *data[100]={0};
	rt_memset(data,0,len*sizeof(uint32_t));
	int result = AFEC_get_data(AFEC_ID,data, len);
	if (result == true)
	{
		rt_kprintf("AFEC_get_data len %d\n",len);
		for (i = 0; i < len; i++)
		{
			rt_kprintf("%07x ",(data[i]!=0)?data[i]-0x200:0);
			if ((i+1)%10 == 0)
				rt_kprintf("\n");
		}
		rt_kprintf("\n");
	}
	else		
		rt_kprintf("AFEC_get_data failed\n");
	//rt_free(data);
}
#ifdef RT_USING_FINSH
#include <finsh.h>
FINSH_FUNCTION_EXPORT(afec_get, test AFEC_get_data);
#endif
