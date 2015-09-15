#include <rthw.h>
#include <rtthread.h>
#include <rtdevice.h>
#include "con_socket.h"

#include "board.h"
//#include <components.h>

#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/uart.h"
#include "driverlib/pin_map.h"
#include "driverlib/interrupt.h"
#include "driverlib/rom_map.h"
#include "driverlib/udma.h"

#include "usblib/usblib.h"
#include "usblib/usbcdc.h"
#include "usblib/usb-ids.h"
#include "usblib/device/usbdevice.h"
#include "usblib/device/usbdcomp.h"
#include "usblib/device/usbdbulk.h"

#include "usb_comp_bulk_structs.h"
extern bool phy_link;
int usb_1=0;
static struct rt_device_usb _hw_usb;
unsigned char *buf;
uint32_t len;
uint8_t g_pucDescriptorData[DESCRIPTOR_DATA_SIZE];
extern uint8_t g_ppui8USBRxBuffer[NUM_BULK_DEVICES][UART_BUFFER_SIZE];
extern uint8_t g_ppcUSBTxBuffer[NUM_BULK_DEVICES][UART_BUFFER_SIZE_TX];
extern struct rt_semaphore rx_sem[4];
extern struct rt_semaphore usbrx_sem[4];
unsigned char check_mem[1+8]					={0xF5,0x8A,0x25,0xff,0x26,0xfa,0x03,0xc3};
unsigned char result_mem[1+8]					={0xF5,0x8B,0x25,0x01,0x26,0xfa,0x00,0x00};


uint8_t *g_usb_rcv_buf[NUM_BULK_DEVICES];
uint32_t send_len;
tDMAControlTable psDMAControlTable[64] __attribute__ ((aligned(1024)));

void USBRxEventCallback(void *pvRxCBData,void **pvBuffer, uint32_t ui32Length);
extern void PinoutSet(bool bEthernet, bool bUSB);
extern int32_t USBBulkTx(void *pvBulkDevice,void *pvBuffer,uint32_t ui32Size);
extern int32_t USBBulkRx(void *pvBulkDevice,void **pvBuffer);

rt_size_t _usb_init()
{

	int i=0;
	PinoutSet(false, true); 
	//MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_UDMA);
    //SysCtlDelay(10);
    //MAP_uDMAControlBaseSet(&psDMAControlTable[0]);
    //MAP_uDMAEnable();
	for(i=0;i<NUM_BULK_DEVICES;i++)
	{
	   //USBBufferInit(&g_sTxBuffer[i]);
	   //USBBufferInit(&g_sRxBuffer[i]);
	   //g_usb_rcv_buf[i]=(uint8_t *)rt_malloc(USB_BUF_LEN);
	   //USBBulkRxBufferOutInit(&g_psBULKDevice[i],(void *)g_usb_rcv_buf[i],USB_BUF_LEN,USBRxEventCallback);
	   g_sCompDevice.psDevices[i].pvInstance = USBDBulkCompositeInit(0, &g_psBULKDevice[i], &g_psCompEntries[i]);
	}
   USBDCompositeInit(0, &g_sCompDevice, DESCRIPTOR_DATA_SIZE,g_pucDescriptorData);

    return 0;
}
int which_usb_device(tUSBDBulkDevice *psDevice)
{
	int i=0;
	for(i=0;i<NUM_BULK_DEVICES;i++)
		if(psDevice==&(g_psBULKDevice[i]))
			break;

	return i;
}
uint32_t
USBTxEventCallback(void *pvCBData, uint32_t ui32Event, uint32_t ui32MsgValue,
          void *pvMsgData)
{
    //
    // We are not required to do anything in response to any transmit event
    // in this example. All we do is update our transmit counter.
    //
         int index=*(int *)pvCBData;
    rt_kprintf("USBTxEventCallback index %d,event %d\n",index,ui32Event);
    if(ui32Event == USB_EVENT_TX_COMPLETE)
    {
		
		send_len=ui32MsgValue;
		rt_sem_release(&(usbrx_sem[index]));
    }
    return(0);
}
uint32_t
USBCommonEventCallback(void *pvCBData, uint32_t ui32Event, uint32_t ui32MsgValue,
          void *pvMsgData)
{
    //
    // Which event are we being sent?
    //
    
	 unsigned char *tmpbuf;
	 int index=*(int *)pvCBData;
	 //rt_kprintf("USBCommonEventCallback index %d, event %d, %d\n",index,ui32Event,ui32MsgValue);
    switch(ui32Event)
    {
        //
        // We are connected to a host and communication is now possible.
        //
        case USB_EVENT_CONNECTED:
        {
            //
            // Flush our buffers.
            //
            //rt_memset(g_usb_rcv_buf[index],'\0',USB_BUF_LEN);
			rt_kprintf("usb connect %d\n",index);
			if(index==1)
				usb_1=1;
            break;
        }

        //
        // The host has disconnected.
        //
        case USB_EVENT_DISCONNECTED:
        {
			rt_kprintf("usb disconnect %d\n",index);
			if(index==1)
				usb_1=0;
            break;
        }

        //
        // A new packet has been received.
        //
        case USB_EVENT_RX_AVAILABLE:
        {
			#if 0
			tmpbuf=rt_malloc(64);
			int bytes=USBBufferRead(&g_sRxBuffer[index],tmpbuf,64);
			if(index!=0)
			{
				rt_kprintf("read index %d ,bytes %d\n",index,bytes);
				//USBBufferWrite(&g_sTxBuffer[index],tmpbuf,bytes);		
				if(phy_link&&(bytes>0)&&g_socket[index-1].connected)
					rt_data_queue_push(&g_data_queue[(index-1)*2], tmpbuf, bytes, RT_WAITING_FOREVER);	
				else
					rt_free(tmpbuf);
			}
			else
			{
				USBBufferWrite(&g_sTxBuffer[index],tmpbuf,bytes);
				rt_free(tmpbuf);
			}
			#endif
			rt_sem_release(&(rx_sem[index]));
			//rt_kprintf("release %d\n",index);
            return 0;
        }

        //
        // Ignore SUSPEND and RESUME for now.
        //
        case USB_EVENT_SUSPEND:
        case USB_EVENT_RESUME:
            break;

        //
        // Ignore all other events and return 0.
        //
        default:
            break;
    }

    return(0);
}
static void _delay_us(uint32_t us)
{
    volatile uint32_t len;
    for (; us > 0; us --)
        for (len = 0; len < 20; len++ );
}

void USBRxEventCallback(void *pvRxCBData,void **pvBuffer, uint32_t ui32Length)
{   

	static int len[5]={0,0,0,0,0};
	static char* ptr[5];
    //return USBBufferRead(&g_sRxBuffer[index],buffer,size);
   	int index=*(int *)pvRxCBData;
	//len[index]=len[index]+ui32Length;
	//if(len[index]==13030400)
	//{
	//	rt_kprintf("usb %d, count %d\n",index,len[index]);
	//	len[index]=0;
	//}
	//return ;
	//int bytes=USBBufferRead(&g_sRxBuffer[index],tmpbuf,64);
	//g_usb_rcv_buf[index]=pvBuffer;
	//uint8_t *tmp=(uint8_t *)rt_malloc(USB_BUF_LEN);
	//USBBulkRxBufferOutInit(&g_psBULKDevice[index],(void *)tmp,USB_BUF_LEN,USBRxEventCallback);
	
	if(ui32Length!=0&&usb_1==1)
	{		
		
		if(index!=0)
		{
			/*if(len[index]==0)
				ptr[index]=*pvBuffer;
			if((len[index]+ui32Length)<1024)
			{
				*pvBuffer=*pvBuffer+ui32Length;
				len[index]+=ui32Length;
				return ;
			}*/
			//USBBufferWrite(&g_sTxBuffer[index],tmpbuf,bytes); 	
			//rt_kprintf("read index %d ,bytes %d,phy_link %d,cnn %d\n",index,ui32Length,phy_link,g_socket[index-1].connected);	
			if(phy_link&&(ui32Length>0)&&g_socket[index-1].connected)
			{
				//unsigned char *tmpbuf=rt_malloc(ui32Length);
				//rt_memcpy(tmpbuf,pvBuffer,ui32Length);
				//send_index_socket(index-1,*pvBuffer,ui32Length);
				//if(rt_data_queue_check_buf(&g_data_queue[(index-1)*2])==RT_EOK)
				{
					rt_data_queue_push(&g_data_queue[(index-1)*2], *pvBuffer, ui32Length, RT_WAITING_FOREVER);				
					//rt_kprintf("push addr %2x bytes %d\r\n",*pvBuffer,ui32Length);
					*pvBuffer=rt_malloc(USB_BUF_LEN);
				}
				//else
				//{

				//}
				//ptr[index]=*pvBuffer;
				//len[index]=0;
			}
			//else
				//rt_free(pvBuffer);
		}
		else
		{
			//USBBufferWrite(&g_sTxBuffer[index],tmpbuf,bytes);
			USBBulkTx(&g_psBULKDevice[index],*pvBuffer,ui32Length);
			//rt_free(tmpbuf);
		}
	}	
}
uint8_t *usbbuf=NULL;

void _usb_read(int dev)
{

	int len,i,crc=0;
	rt_uint8_t *buf;
	len=USBBulkRx(&g_psBULKDevice[dev],&buf);	
	if(dev!=0)
	{
		if(phy_link&&g_socket[dev-1].connected)
		{
			rt_data_queue_push(&g_data_queue[(dev-1)*2],buf, len, RT_WAITING_FOREVER);	
		}
		else
			rt_free(buf);
	}
	else
	{
		//for(i=0;i<8;i++)
		//	rt_kprintf("%x ",buf[i]);
		//rt_kprintf("\nlen %d\n",len);
		if(rt_memcmp(buf,check_mem,8)==0)
		{
			char *tmp=(char *)rt_malloc(USB_BUF_LEN);
			if(tmp!=NULL)
			{
				result_mem[3]=0x01;
				rt_free(tmp);
			}
			else
				result_mem[3]=0x0;
			for(i=0;i<6;i++)
			crc=crc+result_mem[i];
			result_mem[6]=(crc>>8)&0xff;
			result_mem[7]=(crc)&0xff;
			//rt_kprintf("send \n");
			//for(i=0;i<8;i++)
			//rt_kprintf("%x ",result_mem[i]);
			if(USBBulkTx(&g_psBULKDevice[dev],result_mem,8)<0)
				rt_kprintf("usb sent failed\n");;
		}
		//rt_free(buf);
	}
}

int _usb_write(int index, void *buffer, int size)
{
	USBBulkTx(&(g_psBULKDevice[1]),buffer,size);
	return 0;	
}

