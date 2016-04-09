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
//#define COMMAND_FAIL "Command crc fail"
//#define COMMAND_OK "Command exec OK"
char COMMAND_FAIL[]={0xf5,0x8c,0x01};
char COMMAND_OK[]={0xf5,0x8c,0x00};
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
unsigned char config_local_ipv6[] = {0xF5,0x8A,0x20,
							0x12,0x66,0x65,0x38,0x30,0x3a,0x3a,0x31,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
							0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
							0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
							0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
							0x26,0xfa,0x00,0x00};
unsigned char config_remote_ipv6[] = {0xF5,0x8A,0x0c,
							0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
							0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
							0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
							0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
							0x26,0xfa,0x00,0x00};
unsigned char config_local_ip[] 	= {0xF5,0x8A,0x00,0xff,0xff,0xff,0xff,0x26,0xfa,0x00,0x00};
unsigned char config_local_port[] 	= {0xF5,0x8A,0x01,0xff,0xff,0x26,0xfa,0x00,0x00};
unsigned char config_sub_msk[] 		= {0xF5,0x8A,0x05,0xff,0xff,0xff,0xff,0x26,0xfa,0x00,0x00};
unsigned char config_gw[] 			= {0xF5,0x8A,0x06,0xff,0xff,0xff,0xff,0x26,0xfa,0x00,0x00};
unsigned char config_mac[] 			= {0xF5,0x8A,0x07,0xff,0xff,0xff,0xff,0xff,0xff,0x26,0xfa,0x00,0x00};
unsigned char config_remote_ip[] 	= {0xF5,0x8A,0x08,0xff,0xff,0xff,0xff,0x26,0xfa,0x00,0x00};
unsigned char config_remote_port[] 	= {0xF5,0x8A,0x10,0xff,0xff,0x26,0xfa,0x00,0x00};
unsigned char config_net_protol[] 	= {0xF5,0x8A,0x14,0xff,0x26,0xfa,0x00,0x00};
unsigned char config_socket_mode[] 	= {0xF5,0x8A,0x18,0xff,0x26,0xfa,0x00,0x00};
unsigned char config_uart_baud[] 	= {0xF5,0x8A,0x1c,0xff,0x26,0xfa,0x00,0x00};
unsigned char config_tcp[] 			= {0xF5,0x8A,0x21,0xff,0x26,0xfa,0x00,0x00};
unsigned char config_print[]		= {0xF5,0x8B,0x00};
char result_mem[1+8]					={0xF5,0x8B,0x25,0x01,0x26,0xfa,0x02,0xc6};
#define LOCAL_IPADDR_M		"lipaddr"
#define REMOTE_IPADDR_M		"ripaddr"
#define READ				"read"
#define TCP_M				"tcp"
#define SUBMSK_M			"submsk"
#define GW_M				"gw"
#define MAC_M				"mac"
#define RMOTE_PORT_M		"rport"
#define LOCAL_PORT_M		"lport"
#define PROTOL_M			"protol"
#define MODE_M				"mode"
#define LOCAL_IPV6ADDR_M	"lipv6"
#define REMOTE_IPV6ADDR_M	"ripv6"
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
	unsigned char tmp1[256]={0};
	int i;
	printf("CMD:%d\r\n",len);
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
		ret = usb_bulk_read(dev, ep_in, (char *)tmp1, 256, 5000);
		if (ret < 0)
		{
			printf("error 0 reading:\n%s\n", usb_strerror());
		}
		else
		{
			int check_sum=0;
			result=0;
			if(memcmp(tmp1,COMMAND_FAIL,sizeof(COMMAND_FAIL))==0||memcmp(tmp1,COMMAND_OK,sizeof(COMMAND_OK))==0)				
				printf("\r\nACK:\n%02X %02X %02X\n",tmp1[0],tmp1[1],tmp1[2]);
			else
			{
				printf("\r\nACK:%d\n",ret);
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
	}

	return result;
}
int main(int argc, char* argv[])
{
	usb_dev_handle *dev =NULL; /* the device handle */
    unsigned char *cmd;
		char *p;
    int ret,i,len,crc=0;
	char tmp[3]={0};
	char ipf[256]={0};

    usb_init(); /* initialize the library */
    usb_find_busses(); /* find all busses */
    usb_find_devices(); /* find all connected devices */
	#if 0
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
	#else
	if(strcmp(argv[1],LOCAL_IPADDR_M)==0)
	{
		p=argv[2];
		config_local_ip[3]=atoi(p);
		while(*p!='.')
			p++;
		p++;
		config_local_ip[4]=atoi(p);
		while(*p!='.')
			p++;
		p++;
		config_local_ip[5]=atoi(p);
		while(*p!='.')
			p++;
		p++;
		config_local_ip[6]=atoi(p);
		len=sizeof(config_local_ip);
		cmd=config_local_ip;
	}
	else if(strcmp(argv[1],REMOTE_IPADDR_M)==0)
	{
		p=argv[3];
		config_remote_ip[2]=config_remote_ip[2]+atoi(argv[2]);
		config_remote_ip[3]=atoi(p);
		while(*p!='.')
			p++;
		p++;
		config_remote_ip[4]=atoi(p);
		while(*p!='.')
			p++;
		p++;
		config_remote_ip[5]=atoi(p);
		while(*p!='.')
			p++;
		p++;
		config_remote_ip[6]=atoi(p);
		len=sizeof(config_remote_ip);
		cmd=config_remote_ip;
	}
	else if(strcmp(argv[1],READ)==0)
	{
		config_print[2]=atoi(argv[2]);
		len=sizeof(config_print);
		cmd=config_print;
	}
	else if(strcmp(argv[1],TCP_M)==0)
	{
		config_tcp[2]=config_tcp[2]+atoi(argv[2]);
		config_tcp[3]=atoi(argv[3]);
		len=sizeof(config_tcp);
		cmd=config_tcp;
	}
	else if(strcmp(argv[1],SUBMSK_M)==0)
	{
		p=argv[2];
		config_sub_msk[3]=atoi(p);
		while(*p!='.')
			p++;
		p++;
		config_sub_msk[4]=atoi(p);
		while(*p!='.')
			p++;
		p++;
		config_sub_msk[5]=atoi(p);
		while(*p!='.')
			p++;
		p++;
		config_sub_msk[6]=atoi(p);
		len=sizeof(config_sub_msk);
		cmd=config_sub_msk;
	}
	else if(strcmp(argv[1],GW_M)==0)
	{
		p=argv[2];
		config_gw[3]=atoi(p);
		while(*p!='.')
			p++;
		p++;
		config_gw[4]=atoi(p);
		while(*p!='.')
			p++;
		p++;
		config_gw[5]=atoi(p);
		while(*p!='.')
			p++;
		p++;
		config_gw[6]=atoi(p);
		len=sizeof(config_gw);
		cmd=config_gw;
	}
	else if(strcmp(argv[1],MAC_M)==0)
	{
		p=argv[2];
		config_mac[3]=atoi(p);
		while(*p!='-')
			p++;
		p++;
		config_mac[4]=atoi(p);
		while(*p!='-')
			p++;
		p++;
		config_mac[5]=atoi(p);
		while(*p!='-')
			p++;
		p++;
		config_mac[6]=atoi(p);
		while(*p!='-')
			p++;
		p++;
		config_mac[7]=atoi(p);
		len=sizeof(config_mac);
		cmd=config_mac;
	}
	else if(strcmp(argv[1],RMOTE_PORT_M)==0)
	{
		config_remote_port[2]=config_remote_port[2]+atoi(argv[2]);
		config_remote_port[3]=(atoi(argv[3])>>8)&0xff;
		config_remote_port[4]=(atoi(argv[3]))&0xff;
		len=sizeof(config_remote_port);
		cmd=config_remote_port;
	}
	else if(strcmp(argv[1],LOCAL_PORT_M)==0)
	{
		config_local_port[2]=config_local_port[2]+atoi(argv[2]);
		config_local_port[3]=(atoi(argv[3])>>8)&0xff;
		config_local_port[4]=(atoi(argv[3]))&0xff;
		len=sizeof(config_local_port);
		cmd=config_local_port;
	}
	else if(strcmp(argv[1],PROTOL_M)==0)
	{
		config_net_protol[2]=config_net_protol[2]+atoi(argv[2]);
		config_net_protol[3]=atoi(argv[3]);
		len=sizeof(config_net_protol);
		cmd=config_net_protol;
	}
	else if(strcmp(argv[1],MODE_M)==0)
	{
		config_socket_mode[2]=config_socket_mode[2]+atoi(argv[2]);
		config_socket_mode[3]=atoi(argv[3]);
		len=sizeof(config_socket_mode);
		cmd=config_socket_mode;
	}
	else if(strcmp(argv[1],LOCAL_IPV6ADDR_M)==0)
	{
		for(i=0;i<strlen(argv[2]);i++)
			config_local_ipv6[4+i]=argv[2][i];
		config_local_ipv6[3]=strlen(argv[2]);
		len=sizeof(config_local_ipv6);
		cmd=config_local_ipv6;
	}
	else if(strcmp(argv[1],REMOTE_IPV6ADDR_M)==0)
	{
		config_remote_ipv6[2]=config_remote_ipv6[2]+atoi(argv[2]);
		for(i=0;i<strlen(argv[3]);i++)
			config_remote_ipv6[4+i]=argv[3][i];
		config_remote_ipv6[3]=strlen(argv[3]);
		len=sizeof(config_remote_ipv6);
		cmd=config_remote_ipv6;
	}

	if(len!=3)
	{
		for(i=0;i<len-2;i++)
			crc=crc+cmd[i];
		cmd[len-2]=(crc>>8)&0xff;
		cmd[len-1]=(crc)&0xff;
	}
	#endif
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
	send_cmd(dev,(char *)cmd,len);
	//send_cmd(dev,change_ip,11);
	//free(cmd);
	usb_release_interface(dev, intf);
	if (dev)
	{
		usb_close(dev);
	}	
	return 0;
}

