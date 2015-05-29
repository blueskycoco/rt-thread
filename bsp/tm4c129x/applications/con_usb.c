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
uint32_t send_len;
rt_size_t _usb_init()
{

	int i=0;
	PinoutSet(false, true); 
	for(i=0;i<NUM_BULK_DEVICES;i++)
	{
	   USBBufferInit(&g_sTxBuffer[i]);
	   USBBufferInit(&g_sRxBuffer[i]);
	
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
TxHandlerBulk(void *pvCBData, uint32_t ui32Event, uint32_t ui32MsgValue,
          void *pvMsgData)
{
    //
    // We are not required to do anything in response to any transmit event
    // in this example. All we do is update our transmit counter.
    //
    if(ui32Event == USB_EVENT_TX_COMPLETE)
    {
        //g_ui32TxCount += ui32MsgValue;
         int index=which_usb_device((tUSBDBulkDevice *)pvCBData);
		//rt_kprintf("packet sent %d,length %d\n",index,ui32MsgValue);
		send_len=ui32MsgValue;
		rt_sem_release(&(usbrx_sem[index]));
    }
    return(0);
}

uint32_t
RxHandlerBulk(void *pvCBData, uint32_t ui32Event, uint32_t ui32MsgValue,
          void *pvMsgData)
{
    //
    // Which event are we being sent?
    //
    
	 unsigned char *tmpbuf;
     tUSBDBulkDevice *psDevice;
     psDevice = (tUSBDBulkDevice *)pvCBData;
	 int index=which_usb_device(psDevice);
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
            USBBufferFlush(&g_sTxBuffer[index]);
            USBBufferFlush(&g_sRxBuffer[index]);
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

int _usb_read(int index)
{   
    //return USBBufferRead(&g_sRxBuffer[index],buffer,size);
   
		//rt_kprintf("get sem %d\n",index);
		unsigned char *tmpbuf=rt_malloc(64);
		int bytes=USBBufferRead(&g_sRxBuffer[index],tmpbuf,64);
		if(bytes!=0&&usb_1==1)
		{
			if(index!=0)
			{
			//	rt_kprintf("read index %d ,bytes %d\n",index,bytes);
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
		}
		else
			rt_free(tmpbuf);

	
}
static void _delay_us(uint32_t us)
{
    volatile uint32_t len;
    for (; us > 0; us --)
        for (len = 0; len < 20; len++ );
}

int _usb_write(int index, void *buffer, int size)
{
	int len=0,len_out=0,tmp_size=size,size64=64,send_size=0,addr=0;
	//rt_kprintf("_usb_write index %d,size %d\n",index,size);
	//send_size=USBBufferWrite(&g_sTxBuffer[index],buffer,size);
	//rt_sem_take(&(usbrx_sem[index]), RT_WAITING_FOREVER);
	//rt_kprintf("_usb_write %d %d %d\n",size,send_size,send_len);
	//return 0;
	//len_out=USBBufferSpaceAvailable(&g_sTxBuffer[index]);
	//rt_kprintf("===>%d %d\n",size,len_out);
	if(index==3)
		index=2;
	else if(index==5)
		index=3;
	else if(index==7)
		index=4;
	while(tmp_size!=0)
	{
		len_out=USBBufferSpaceAvailable(&g_sTxBuffer[index]);
		if(len_out==0)
		{
			//rt_thread_delay(1);
			_delay_us(1);
		}
		else
		{
			if(tmp_size>len_out)
				send_size=len_out;
			else
				send_size=tmp_size;
			USBBufferWrite(&g_sTxBuffer[index],buffer+addr,send_size);
			//rt_sem_take(&(usbrx_sem[index]), RT_WAITING_FOREVER);
			addr=addr+send_size;
			tmp_size=tmp_size-send_size;
			//rt_kprintf("tmp_size %d,send_size %d\n",tmp_size,send_size);
		}
	}
	//rt_kprintf("<===\n");
	return 0;
	int loop=size/64;
	int last_bytes=size%64;
	//rt_kprintf("_usb_write %d loop %d ,last_bytes %d\n",size,loop,last_bytes);
	int i;
	for(i=0;i<loop;i++)
	{
		USBBufferWrite(&g_sTxBuffer[index],buffer+i*64,64);
		rt_sem_take(&(usbrx_sem[index]), RT_WAITING_FOREVER);
		//rt_kprintf("64 len_out %d\n",send_len);
	}
	if(last_bytes!=0)
	{
		USBBufferWrite(&g_sTxBuffer[index],buffer+i*64,last_bytes);
		rt_sem_take(&(usbrx_sem[index]), RT_WAITING_FOREVER);
		//rt_kprintf("len_out %d\n",send_len);
	}
		return 0;
	while(tmp_size!=0)
	{
		len_out=USBBufferWrite(&g_sTxBuffer[index],buffer+len,size64);
		if(rt_sem_take(&(usbrx_sem[index]), RT_WAITING_FOREVER) != RT_EOK) continue;
		if(send_len!=64)
		{
			rt_kprintf("len_out %d\n",send_len);
			break;
		}
		if(tmp_size>64)
			tmp_size=tmp_size-64;
		else
			size64=tmp_size;
		
		len=len+size64;
	}
	return 0;
	while(1)
	{
	    len_out=USBBufferWrite(&g_sTxBuffer[index],buffer+len,tmp_size);
		/*if(len_out==0)
		{
			_delay_us(1);
			continue;
		}*/
		
		if(rt_sem_take(&(usbrx_sem[index]), RT_WAITING_FOREVER) != RT_EOK) continue;
		if(len_out!=tmp_size)
		{
			//rt_kprintf("len_out %d , tmp_size %d\n",len_out,tmp_size);
			len=len+len_out;
			tmp_size=tmp_size-len_out;			
		}
		else
			break;
		
	}

	return 0;
}

