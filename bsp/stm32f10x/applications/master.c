#include <board.h>
#include <rtthread.h>
#include <rtdevice.h>
#include <dfs_posix.h>
#include "master.h"
#include "lcd.h"
#define FQP_FILE	"/fqp.dat"
#define MP_FILE		"/mp.dat"
#define MAIN_STATION_PROTECT_ON 1
#define MAIN_STATION_PROTECT_OFF 0
#define DEFAULT_DOMAIN "kjfslkjflskdjfj"
struct rt_event g_info_event;
void dump_mp()
{
	int i;
	rt_kprintf("\r\n\r\nMainStation Param ......\r\n");
	rt_kprintf("socketAddressVersion \t%d\r\n",
		mp.socketAddressVersion);
	rt_kprintf("appVersion \t\t%d\r\n",
		mp.appVersion);
	rt_kprintf("socketDomainVersion \t%d\r\n",
		mp.socketDomainVersion);
	rt_kprintf("updateAddressVersion \t%d\r\n",
		mp.updateAddressVersion);
	rt_kprintf("updateDomainVersion \t%d\r\n",
		mp.updateDomainVersion);
	rt_kprintf("firmCRC \t\t%d\r\n",
		mp.firmCRC);
	rt_kprintf("firmLength \t\t%d\r\n",
		mp.firmLength);
	rt_kprintf("firmVersion \t\t%d\r\n",
		mp.firmVersion);
	rt_kprintf("status \t\t\t%d\r\n",
		mp.status);
	rt_kprintf("CAPTCHA \t\t%x%x%x%x%x%x\r\n",
		mp.roProperty.CAPTCHA[0],
		mp.roProperty.CAPTCHA[1],
		mp.roProperty.CAPTCHA[2],
		mp.roProperty.CAPTCHA[3],
		mp.roProperty.CAPTCHA[4],
		mp.roProperty.CAPTCHA[5]);
	rt_kprintf("model \t\t\t%x\r\n",
		mp.roProperty.model);
	rt_kprintf("sn \t\t\t%x%x%x%x%x%x\r\n",
		mp.roProperty.sn[0],
		mp.roProperty.sn[1],
		mp.roProperty.sn[2],
		mp.roProperty.sn[3],
		mp.roProperty.sn[4],
		mp.roProperty.sn[5]);
	for (i=0;i<10;i++) {
		if (mp.socketAddress[i].IP[0] != 0) {
			rt_kprintf("socket IP[%d] \t\t%d.%d.%d.%d\r\n",i,
			mp.socketAddress[i].IP[0],
			mp.socketAddress[i].IP[1],
			mp.socketAddress[i].IP[2],
			mp.socketAddress[i].IP[3]);
			rt_kprintf("socket IP port \t\t%d\r\n",
				mp.socketAddress[i].port);
		}
	}
	rt_kprintf("socket domain \t\t%s\r\n", 
		mp.socketDomainAddress.domain);
	rt_kprintf("socket domain port \t%d\r\n",
		mp.socketDomainAddress.port);
	rt_kprintf("update IP \t\t%d.%d.%d.%d\r\n",
			mp.updateAddress.IP[0],
			mp.updateAddress.IP[1],
			mp.updateAddress.IP[2],
			mp.updateAddress.IP[3]);
	rt_kprintf("update IP port \t\t%d\r\n",
			mp.updateAddress.port);	
	rt_kprintf("update domain \t\t%s\r\n", 
		mp.updateDomainAddress.domain);
	rt_kprintf("update domain port \t%d\r\n",
		mp.updateDomainAddress.port);
	rt_kprintf("\r\n\r\n");
}
int load_param()
{
	int length;
	int fd = open(MP_FILE, O_RDONLY, 0);
	if (fd < 0)
	{
		rt_kprintf("INIT factory\n");
		rt_memset(&mp, 0, sizeof(struct MachineProperty));
		mp.socketAddressVersion= 0;
		mp.socketDomainVersion= 0;
		mp.updateAddressVersion = 0;
		mp.updateDomainVersion = 0;
		mp.appVersion = 0;
		mp.firmCRC = 0;
		mp.firmLength = 0;
		mp.firmVersion = 1;
		mp.status = MAIN_STATION_PROTECT_OFF;
		rt_memset(&mp.roProperty.CAPTCHA ,0,6);
		mp.roProperty.model = 0;
		mp.roProperty.sn[5] = 0x34;;
		mp.roProperty.sn[4] = 0x12;
		mp.roProperty.sn[3] = 0x02;
		mp.roProperty.sn[2] = 0x01;
		mp.roProperty.sn[1] = 0x18;
		mp.roProperty.sn[0] = 0xa1;
		mp.socketAddress[0].IP[0] = 101;
		mp.socketAddress[0].IP[1] = 132;
		mp.socketAddress[0].IP[2] = 177;
		mp.socketAddress[0].IP[3] = 116;
		mp.socketAddress[0].port = 2011;
		strcpy(mp.socketDomainAddress.domain,DEFAULT_DOMAIN);
		mp.socketDomainAddress.port = 2011;		
		mp.updateAddress.IP[0] = 101;
		mp.updateAddress.IP[1] = 132;
		mp.updateAddress.IP[2] = 177;
		mp.updateAddress.IP[3] = 116;
		mp.updateAddress.port = 2011;
		strcpy(mp.updateDomainAddress.domain,DEFAULT_DOMAIN);
		mp.updateDomainAddress.port = 2011;		
		fd = open(MP_FILE, O_WRONLY | O_CREAT | O_TRUNC, 0);
		length = write(fd, &mp, sizeof(mp));
		if (length != sizeof(mp))
		{
			rt_kprintf("write mp data failed\n");
			close(fd);
			return 0;
		}
	} else {
		rt_kprintf("read mp data\r\n");
		length = read(fd, &mp, sizeof(mp));
		if (length != sizeof(mp))
		{
			rt_kprintf("check: read mp data failed\n");
			close(fd);
			return 0;
		}
		dump_mp();
	}
	close(fd);

	fd = open(FQP_FILE, O_RDONLY, 0);
	if (fd > 0)
	{		
		rt_memset(&fqp, 0, sizeof(struct FangQuProperty));
		rt_kprintf("read fqp data\r\n");
		length = read(fd, &fqp, sizeof(fqp));
		if (length != sizeof(fqp))
		{
			rt_kprintf("check: read fqp data failed\n");
			close(fd);
			return 0;
		}
	}
	close(fd);
	return 1;
}

void info_user(void *param)
{
	rt_uint32_t ev;
	
	while (1) {
		rt_event_recv( &(g_info_event), 0xffffffff, RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR, RT_WAITING_FOREVER, &ev ); 
		if (ev & INFO_EVENT_CODING) {
			SetErrorCode(0x01);			
		}
		if (ev & INFO_EVENT_NORMAL) {
			SetErrorCode(0x00);			
		}
		if (ev & INFO_EVENT_FACTORY_RESET) {
			SetErrorCode(0x02);			
		}
	}
}
