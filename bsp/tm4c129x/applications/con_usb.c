#include <rthw.h>
#include <rtthread.h>
#include <rtdevice.h>
#include "con_socket.h"
#include "con_common.h"
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

extern void PinoutSet(bool bEthernet, bool bUSB);
extern int32_t USBBulkTx(void *pvBulkDevice,void *pvBuffer,uint32_t ui32Size);
extern int32_t USBBulkRx(void *pvBulkDevice,void **pvBuffer);

rt_size_t _usb_init()
{

	int i=0;
	PinoutSet(false, true); 
	for(i=0;i<NUM_BULK_DEVICES;i++)
	{
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
		//rt_sem_release(&(usbrx_sem[index]));
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
void _usb_read(int dev)
{

	int len,i,crc=0;
	rt_uint8_t *buf;
	len=USBBulkRx(&g_psBULKDevice[dev],(void **)&buf);	
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
		else
		{
			if(buf[0]==0xf5 && buf[1]==0x8a)
			{		
				int check_sum=0,longlen=0;
				rt_kprintf("Usb Config len %d\r\n",len);
				for(i=0;i<len-2;i++)
				{
					rt_kprintf("%02x ",buf[i]);
					check_sum+=buf[i];
				}
				rt_kprintf("\r\n");
				if(check_sum==(buf[len-2]<<8|buf[len-1]))
				{
					if(buf[2]==0x0c || buf[2]==0x0d || buf[2]==0x0e || buf[2]==0x0f || buf[2]==0x20)
						longlen=buf[3];					
					//usb_config(buf+2,longlen,0);
					set_config(buf+2,longlen,0);
					USBBulkTx(&g_psBULKDevice[dev],(void *)COMMAND_OK, strlen(COMMAND_OK));
				}
				else
					USBBulkTx(&g_psBULKDevice[dev],(void *)COMMAND_FAIL, strlen(COMMAND_FAIL));
			}
			else if(buf[0]==0xf5 && buf[1]==0x8b)
			{
				rt_int32_t lenout;
				char *tmp=send_out(dev,buf[2],&lenout);
				if(tmp!=NULL)
				{
					int ii=0;
					for(ii=0;ii<lenout;ii++)
						rt_kprintf("%2x ",tmp[ii]);
					USBBulkTx(&g_psBULKDevice[dev],(void *)tmp, lenout);
				}
				else
				{
					rt_kprintf("some error\r\n");
					USBBulkTx(&g_psBULKDevice[dev],(void *)COMMAND_FAIL, strlen(COMMAND_FAIL));
				}
			}
			else
				USBBulkTx(&g_psBULKDevice[dev],check_mem,8);
		}
	}
}

int _usb_write(int index, void *buffer, int size)
{
	USBBulkTx(&(g_psBULKDevice[1]),buffer,size);
	return 0;	
}

void usb_w_thread(void* parameter)
{
	rt_int32_t dev=((rt_int32_t)parameter)/2;
	rt_kprintf("usb_w_thread %d\r\n",dev);
	while(1)
		_usb_read(dev);
}
/*
void usb_config(rt_uint8_t *data,rt_int32_t ipv6_len,rt_int32_t dev)
{
	set_config(data,ipv6_len,dev);
	print_config(g_conf);
	void *ptr1=(void *)&g_confb;
	void *ptr2=(void *)&g_conf;
	if(rt_memcmp(ptr1,ptr2,sizeof(config))!=0)
	{
		print_config(g_conf);
	}

}
*/
