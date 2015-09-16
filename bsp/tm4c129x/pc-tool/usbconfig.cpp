// usbtest.cpp : Defines the entry point for the console application.
//

//#include "stdafx.h"
#include "lusb0_usb.h"
#include <stdio.h>
#include <time.h> 
#include <wchar.h>
//#include <sys/timeb.h>
#define MY_VID 0x1cbe
#define MY_PID 0x0003

// Device configuration and interface id.
#define MY_CONFIG 1
#define MY_INTF 4

// Device endpoint(s)
#define EP_IN 0x85
#define EP_OUT 0x05
int intf=0;
int ep_in=0x81;
int ep_out=0x01;
// Device of bytes to transfer.
#define BUF_SIZE 16384
char check_mem[1+8]					={0xF5,0x8A,0x25,0xff,0x26,0xfa,0x03,0xc3};
char change_ip[1+11]					={0xF5,0x8A,0x00,0xc0,0xa8,0x01,0xdf,0x26,0xfa,0x04,0xe7};

char result_mem[1+8]					={0xF5,0x8B,0x25,0x01,0x26,0xfa,0x02,0xc6};

usb_dev_handle *open_dev(void);

static int transfer_bulk_async(usb_dev_handle *dev,
                               int ep,
                               char *bytes,
                               int size,
                               int timeout);

usb_dev_handle *open_dev(int index)
{
    struct usb_bus *bus;
    struct usb_device *dev;

    for (bus = usb_get_busses(); bus; bus = bus->next)
    {
        for (dev = bus->devices; dev; dev = dev->next)
        {
			//printf("filename %s\n",dev->filename);
            if (dev->descriptor.idVendor == MY_VID
                    && dev->descriptor.idProduct == MY_PID
					&& dev->config->interface->altsetting->bInterfaceNumber==intf)
            {
                return usb_open(dev);
            }
        }
    }
    return NULL;
}
void TcharToChar (const TCHAR * tchar, char ** _char)  
{  
    int iLength ;  
	//获取字节长度   
	iLength = WideCharToMultiByte(CP_ACP, 0, (const WCHAR *)tchar, -1, NULL, 0, NULL, NULL);  
	//printf("iLength %d\n",iLength);
	*_char=(char *)malloc(iLength);
	memset(*_char,'\0',iLength);
	//将tchar值赋给_char    
	WideCharToMultiByte(CP_ACP, 0, (const WCHAR *)tchar, -1, *_char, iLength, NULL, NULL);   
	printf("%s %d\n",*_char,iLength);
}  
#define W_OP 1
int send_cmd(usb_dev_handle *dev,char *cmd,int len)
{
	int ret,result=0;
	unsigned char tmp1[32]={0};
	int i;
	printf("CMD:\r\n");
	for(i=0;i<len;i++)
		printf("%02x ",((unsigned char *)cmd)[i]);
	printf("\r\n");
	ret = usb_bulk_write(dev, ep_out, cmd, len, 5000);
	if (ret < 0)
	{
		printf("error 0 writing:\n%s\n", usb_strerror());
	}
	else
	{
		ret = usb_bulk_read(dev, ep_in, (char *)tmp1, 32, 5000);
		if (ret < 0)
		{
			printf("error 0 reading:\n%s\n", usb_strerror());
		}
		else
		{
			int check_sum=0;
			result=0;
			printf("\r\nACK:\n");
			for(i=0;i<ret;i++)
			printf("%02x ",tmp1[i]);
			for(i=0;i<ret-2;i++)
				check_sum+=tmp1[i];
			if(check_sum==(tmp1[ret-2]<<8|tmp1[ret-1]))
				printf("\r\nCheck Sum OK!\r\n");
			else
				printf("\r\nCheck Sum Failed!%04x\r\n",tmp1[ret-2]<<8|tmp1[ret-1]);
		}
	}

	return result;
}
int main(int argc, char* argv[])
{
	usb_dev_handle *dev =NULL; /* the device handle */
    char *cmd;
    int ret,i,len;
	char tmp[3]={0};

    usb_init(); /* initialize the library */
    usb_find_busses(); /* find all busses */
    usb_find_devices(); /* find all connected devices */
	len=atoi(argv[1]);
	cmd=(char *)malloc(len/2+1);
	memset(cmd,0,len/2);
	for(i=0;i<len;i=i+2)
	{
		memcpy(tmp,(const char *)(argv[2]+i),2);
		//TcharToChar(tmp,&index);
		if(tmp[0]>='a')
			cmd[i/2]+=(tmp[0]-'a'+10)*16;
		else
			cmd[i/2]+=(tmp[0]-'0')*16;
		if(tmp[1]>='a')
			cmd[i/2]+=tmp[1]-'a'+10;
		else
			cmd[i/2]+=tmp[1]-'0';
	}
	len=len/2;
    if (!(dev = open_dev(0)))
    {
        printf("error opening device: \n%s\n", usb_strerror());
        return 0;
    }
    else
    {
        //printf("success: device %04X:%04X opened\n", MY_VID, MY_PID);
    }
	 if (usb_set_configuration(dev, MY_CONFIG) < 0)
    {
        printf("error setting config #%d: %s\n", MY_CONFIG, usb_strerror());
        usb_close(dev);
        return 0;
    }
    else
    {
        //printf("success: set configuration #%d\n", MY_CONFIG);
    }
	 
	 if (usb_claim_interface(dev, intf) < 0)
    {
        printf("error claiming interface #%d:\n%s\n", intf, usb_strerror());
        usb_close(dev);
        return 0;
    }
    else
    {
        //printf("success: claim_interface #%d\n", intf);
    }

	 ret = usb_control_msg(dev, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN,
                          14, /* set/get test */
                          2,  /* test type    */
                          intf,  /* interface id */
                          tmp, 1, 1000);
	send_cmd(dev,cmd,len);
	//send_cmd(dev,change_ip,11);
	free(cmd);
	usb_release_interface(dev, intf);
	if (dev)
	{
		usb_close(dev);
	}	
	return 0;
}

