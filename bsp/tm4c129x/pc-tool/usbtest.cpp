// usbtest.cpp : Defines the entry point for the console application.
//

//#include "stdafx.h"
#include "lusb0_usb.h"
#include <stdio.h>
#include <time.h> 
//#include <sys/timeb.h>
#define MY_VID 0x1cbe
#define MY_PID 0x0003

// Device configuration and interface id.
#define MY_CONFIG 1
#define MY_INTF 4

// Device endpoint(s)
#define EP_IN 0x85
#define EP_OUT 0x05
int intf[5]={0,1,2,3,4};
int ep_in[5]={0x81,0x82,0x83,0x84,0x85};
int ep_out[5]={0x01,0x02,0x03,0x04,0x05};
// Device of bytes to transfer.
#define BUF_SIZE 64*1024
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
			printf("filename %s\n",dev->filename);
            if (dev->descriptor.idVendor == MY_VID
                    && dev->descriptor.idProduct == MY_PID
					&& dev->config->interface->altsetting->bInterfaceNumber==intf[index])
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
//printf("%s\n",*_char);
}  
#define W_OP 1
int main(int argc, _TCHAR* argv[])
{
	usb_dev_handle *dev[5] ={NULL,NULL,NULL,NULL,NULL}; /* the device handle */
    char tmp[BUF_SIZE],tmp1[BUF_SIZE],*index,*file_out,*file_in;
    int ret,i;
	time_t ltime1, ltime2;
    void* async_read_context = NULL;
    void* async_write_context = NULL;

    usb_init(); /* initialize the library */
    usb_find_busses(); /* find all busses */
    usb_find_devices(); /* find all connected devices */
	for(i=0;i<BUF_SIZE;i++)
		tmp[i]=i;
	TcharToChar(argv[1],&index);
	i=atoi(index);
	printf("open usb device %d \n",i);
#if !W_OP
	TcharToChar(argv[2],&file_out);
	HANDLE hFile_out = CreateFile((LPCWSTR)(argv[2]),
        GENERIC_WRITE,  
        FILE_SHARE_WRITE,  
        NULL,  
        CREATE_ALWAYS,  
        0,  
        NULL );
	if(hFile_out!=INVALID_HANDLE_VALUE)
		 printf("Create %s ok\n",file_out);
	 else
		 printf("Can not open %s\n",file_out);
#else
	TcharToChar(argv[2],&file_in);
	 HANDLE hFile_in = CreateFile((const CHAR *)(LPCWSTR)(argv[2]),  
        GENERIC_READ,  
        FILE_SHARE_READ,  
        NULL,  
        OPEN_EXISTING,  
        0,  
        NULL ); 
	  if(hFile_in!=INVALID_HANDLE_VALUE)
		 printf("Open %s ok\n",file_in);
	 else
		 printf("Can not open %s\n",file_in);
#endif	  
    if (!(dev[i] = open_dev(i)))
    {
        printf("error opening device: \n%s\n", usb_strerror());
        return 0;
    }
    else
    {
        printf("success: device %04X:%04X opened\n", MY_VID, MY_PID);
    }
	 if (usb_set_configuration(dev[i], MY_CONFIG) < 0)
    {
        printf("error setting config #%d: %s\n", MY_CONFIG, usb_strerror());
        usb_close(dev[i]);
        return 0;
    }
    else
    {
        printf("success: set configuration #%d\n", MY_CONFIG);
    }
	 
	 if (usb_claim_interface(dev[i], intf[i]) < 0)
    {
        printf("error claiming interface #%d:\n%s\n", intf[i], usb_strerror());
        usb_close(dev[i]);
        return 0;
    }
    else
    {
        printf("success: claim_interface #%d\n", intf[i]);
    }

	 ret = usb_control_msg(dev[i], USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN,
                          14, /* set/get test */
                          2,  /* test type    */
                          intf[i],  /* interface id */
                          tmp, 1, 1000);
	 
	 DWORD nBytes; 
	int bytes_w=0,bytes_r=0;	
	BOOL bResult;  
	time (&ltime1);
	while(1)
	{	
#if W_OP
		bResult = ReadFile(hFile_in,tmp,sizeof(tmp),&nBytes,NULL);
		if(nBytes>0)
		{
			ret = usb_bulk_write(dev[i], ep_out[i], tmp, nBytes, 5000);
			if (ret < 0)
			{
				printf("error writing:\n%s\n", usb_strerror());
				printf("success: bulk write %d bytes\n", bytes_w);
			}
			else
			{
				bytes_w=bytes_w+ret;
				// printf("usb %d bulk read %d bytes\n",i, bytes_r);/*, bulk read %d bytes*/
			}
		}
		else
			break;
#else
		ret = usb_bulk_read(dev[i], ep_in[i], tmp1, sizeof(tmp1), 5000);
		if (ret < 0)
		{
			printf("error reading:\n%s\n %d", usb_strerror(),bytes_r);
			printf("success: bulk read %d bytes\n", bytes_r);
		}
		else
		{
			bytes_r=bytes_r+ret;
			WriteFile( hFile_out, tmp1, ret, &nBytes, NULL ); 
			printf("usb %d bulk read %d bytes\n",i, bytes_r);/*, bulk read %d bytes*/
		}
		//printf("usb %d bulk write %d bytes, bulk read %d bytes\n",i, bytes_w,bytes_r);
#endif	
			
	}
	time (&ltime2);
#if !W_OP
	CloseHandle(hFile_out);  	
	if(file_out)
		free(file_out);
#else 
	CloseHandle(hFile_in);
	if(file_in)
		free(file_in);
#endif
	//if(index)
	//	free(index);
	
	usb_release_interface(dev[i], intf[i]);
	if (dev[i])
	{
		usb_close(dev[i]);
	}	
    printf("Done.r %d, w %d \r\n",bytes_r,bytes_w);
	if(ltime2-ltime1>0)
		printf("%f MBytes/s\n",((float)bytes_w)/((float)(((ltime2-ltime1)*1024*1024))));
	else
		printf("1.25 Mbytes/s\n");
	return 0;
}

