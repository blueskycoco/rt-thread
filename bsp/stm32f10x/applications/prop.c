#include <board.h>
#include <rtthread.h>
#include <rtdevice.h>
#include <dfs_posix.h>
#include "prop.h"
#include "lcd.h"
#include "led.h"
#include "bsp_misc.h"
#include "wtn6.h"

struct rt_mutex file_lock;

void dump_mp(struct MachineProperty v)
{
	int i;
	rt_kprintf("\r\n\r\n<<MainStation Param>>\r\n");
	rt_kprintf("socketAddressVersion \t%d\r\n",
		v.socketAddressVersion);
	rt_kprintf("appVersion \t\t%d\r\n",
		v.appVersion);
	rt_kprintf("socketDomainVersion \t%d\r\n",
		v.socketDomainVersion);
	rt_kprintf("updateAddressVersion \t%d\r\n",
		v.updateAddressVersion);
	rt_kprintf("updateDomainVersion \t%d\r\n",
		v.updateDomainVersion);
	rt_kprintf("firmCRC \t\t%d\r\n",
		v.firmCRC);
	rt_kprintf("firmLength \t\t%d\r\n",
		v.firmLength);
	rt_kprintf("firmVersion \t\t%d\r\n",
		v.firmVersion);
	rt_kprintf("status \t\t\t%d\r\n",
		v.status);
	rt_kprintf("CAPTCHA \t\t%x%x%x%x%x%x\r\n",
		v.roProperty.CAPTCHA[0],
		v.roProperty.CAPTCHA[1],
		v.roProperty.CAPTCHA[2],
		v.roProperty.CAPTCHA[3],
		v.roProperty.CAPTCHA[4],
		v.roProperty.CAPTCHA[5]);
	rt_kprintf("model \t\t\t%x\r\n",
		v.roProperty.model);
	rt_kprintf("sn \t\t\t%x%x%x%x%x%x\r\n",
		v.roProperty.sn[0],
		v.roProperty.sn[1],
		v.roProperty.sn[2],
		v.roProperty.sn[3],
		v.roProperty.sn[4],
		v.roProperty.sn[5]);
	for (i=0;i<10;i++) {
		if (v.socketAddress[i].IP[0] != 0) {
			rt_kprintf("socket IP[%d] \t\t%d.%d.%d.%d\r\n",i,
			v.socketAddress[i].IP[0],
			v.socketAddress[i].IP[1],
			v.socketAddress[i].IP[2],
			v.socketAddress[i].IP[3]);
			rt_kprintf("socket IP port \t\t%d\r\n",
				v.socketAddress[i].port);
		}
	}
	rt_kprintf("socket domain \t\t%s\r\n", 
		v.socketDomainAddress.domain);
	rt_kprintf("socket domain port \t%d\r\n",
		v.socketDomainAddress.port);
	rt_kprintf("update IP \t\t%d.%d.%d.%d\r\n",
			v.updateAddress.IP[0],
			v.updateAddress.IP[1],
			v.updateAddress.IP[2],
			v.updateAddress.IP[3]);
	rt_kprintf("update IP port \t\t%d\r\n",
			v.updateAddress.port);	
	rt_kprintf("update domain \t\t%s\r\n", 
		v.updateDomainAddress.domain);
	rt_kprintf("update domain port \t%d\r\n",
		v.updateDomainAddress.port);
	rt_kprintf("ccid \t\t\t");
	for(i=0;i<10;i++)
		rt_kprintf("%02x", v.qccid[i]);
	rt_kprintf("\r\n\r\n");
}
void dump_fqp(struct FangQuProperty v1, struct FangQu *v2,struct FangQu *v3)
{	
	int i;
	rt_kprintf("\r\n\r\n<<FangQu Param>>\r\n");
	rt_kprintf("status \t%d\r\n",v1.status);
	rt_kprintf("alarm_voice \t%d\r\n",v1.alarm_voice_time);
	rt_kprintf("delay_in \t%d\r\n",v1.delay_in);
	rt_kprintf("delya_out \t%d\r\n",v1.delya_out);
	rt_kprintf("auto_bufang \t%08x\r\n",v1.auto_bufang);
	rt_kprintf("auto_chefang \t%08x\r\n",v1.auto_chefang);
	rt_kprintf("is_alarm_voice \t%d\r\n",v1.is_alarm_voice);
	rt_kprintf("is_check_AC \t%d\r\n",v1.is_check_AC);
	rt_kprintf("is_check_DC \t%d\r\n",v1.is_check_DC);
	rt_kprintf("is_lamp \t%d\r\n",v1.is_lamp);
	rt_kprintf("PGM \t\t%02x%02x\r\n",v1.PGM0,v1.PGM1);
	
	rt_kprintf("wire fq\r\n");
	for(i=0;i<WIRE_MAX;i++)
	{
		if(v2[i].index != 0) {
			rt_kprintf("FangQu[%d]\r\n",
				v2[i].index);
			rt_kprintf("type \t%d\r\n",v2[i].type);
			rt_kprintf("operationType \t%d\r\n",v2[i].operationType);
			rt_kprintf("voiceType \t%d\r\n",v2[i].voiceType);
			rt_kprintf("alarmType \t%d\r\n",v2[i].alarmType);
			rt_kprintf("isBypass \t%d\r\n",v2[i].isBypass);			
			rt_kprintf("isStay \t%d\r\n",v2[i].isStay);
			rt_kprintf("status \t%d\r\n",v2[i].status);
			rt_kprintf("slave_delay \t%d\r\n",v2[i].slave_delay);
			rt_kprintf("slave_type \t%x\r\n",v2[i].slave_type);
			rt_kprintf("slave_model \t%x\r\n",v2[i].slave_model);
			rt_kprintf("slave_batch \t%x\r\n",v2[i].slave_batch);
			rt_kprintf("slave_sn \t%08x\r\n",v2[i].slave_sn);
		}
	}

	rt_kprintf("wireless fq\r\n");
	for(i=0;i<WIRELESS_MAX;i++)
	{
		if(v3[i].index != 0) {
			rt_kprintf("FangQu[%d]\r\n",
				v3[i].index);
			rt_kprintf("type \t%d\r\n",v3[i].type);
			rt_kprintf("operationType \t%d\r\n",v3[i].operationType);
			rt_kprintf("voiceType \t%d\r\n",v3[i].voiceType);
			rt_kprintf("alarmType \t%d\r\n",v3[i].alarmType);
			rt_kprintf("isBypass \t%d\r\n",v3[i].isBypass);			
			rt_kprintf("isStay \t%d\r\n",v3[i].isStay);
			rt_kprintf("status \t%d\r\n",v3[i].status);
			rt_kprintf("slave_delay \t%d\r\n",v3[i].slave_delay);
			rt_kprintf("slave_type \t%x\r\n",v3[i].slave_type);
			rt_kprintf("slave_model \t%x\r\n",v3[i].slave_model);
			rt_kprintf("slave_batch \t%x\r\n",v3[i].slave_batch);
			rt_kprintf("slave_sn \t%08x\r\n",v3[i].slave_sn);
		}
	}
}

int load_param()
{
	int length;
	rt_uint16_t crc;
	rt_uint16_t tmp_crc;
	struct MachineProperty tmp_mp;
	struct FangQuProperty tmp_fqp;
	struct FangQu *tmp_fangquList;
	rt_mutex_init(&file_lock,	"file_lock",	RT_IPC_FLAG_FIFO);

	rt_memset(&fqp, 0, sizeof(fqp));
	rt_memset(fangqu_wire, 0, sizeof(struct FangQu)*WIRE_MAX);
	rt_memset(fangqu_wireless, 0, sizeof(struct FangQu)*WIRELESS_MAX);
	rt_memset(&mp, 0, sizeof(struct MachineProperty));
	
	mp.socketAddressVersion= 0;
	mp.socketDomainVersion= 0;
	mp.updateAddressVersion = 0;
	mp.updateDomainVersion = 0;
	mp.appVersion = 0;
	mp.firmCRC = 0;
	mp.firmLength = 0;
	mp.firmVersion = 1;
	mp.status = 0;
	rt_memset(&mp.roProperty.CAPTCHA ,0,6);
	mp.roProperty.model = 0;
	mp.roProperty.sn[5] = 0x45;
	mp.roProperty.sn[4] = 0x23;
	mp.roProperty.sn[3] = 0x21;
	mp.roProperty.sn[2] = 0x01;
	mp.roProperty.sn[1] = 0x18;
	mp.roProperty.sn[0] = 0xa1;
	mp.socketAddress[0].IP[0] = 101;
	mp.socketAddress[0].IP[1] = 132;
	mp.socketAddress[0].IP[2] = 177;
	mp.socketAddress[0].IP[3] = 116;
	mp.socketAddress[0].port = 1705;
	strcpy(mp.socketDomainAddress.domain,DEFAULT_DOMAIN);
	mp.socketDomainAddress.port = 2011;
	mp.updateAddress.IP[0] = 101;
	mp.updateAddress.IP[1] = 132;
	mp.updateAddress.IP[2] = 177;
	mp.updateAddress.IP[3] = 116;
	mp.updateAddress.port = 2011;
	strcpy(mp.updateDomainAddress.domain,DEFAULT_DOMAIN);
	mp.updateDomainAddress.port = 2011;

	fqp.alarm_voice_time=DEFAULT_VOICE_TIME;
	fqp.auto_bufang=0;
	fqp.auto_chefang=0;
	fqp.delay_in=DEFAULT_DELAY_IN;
	fqp.delya_out=DEFAULT_DELAY_OUT;
	fqp.is_alarm_voice=DEFAULT_ALARM_VOICE;
	fqp.is_check_AC=DEFAULT_CHECK_AC;
	fqp.is_check_DC=DEFAULT_CHECK_DC;
	fqp.is_lamp = 0x04;
	fqp.PGM0=0;
	fqp.PGM1=1;
	fqp.status=0;

	int fd = open(MP_FILE, O_RDONLY, 0);
	if (fd < 0)
	{
		rt_kprintf("INIT factory\n");
		fd = open(MP_FILE, O_WRONLY | O_CREAT | O_TRUNC, 0);
		rt_uint16_t crc = CRC_check((unsigned char *)&mp, sizeof(mp));
		rt_kprintf("init crc %x\r\n", crc);
		write(fd, &crc, sizeof(rt_uint16_t));
		length = write(fd, &mp, sizeof(mp));
		if (length != sizeof(mp))
		{
			rt_kprintf("write mp data failed\n");
			close(fd);
			return 0;
		}
	} else {
		rt_kprintf("read mp data\r\n");
		read(fd, &crc, sizeof(rt_uint16_t));
		length = read(fd, &tmp_mp, sizeof(tmp_mp));
		//dump_mp(tmp_mp);
		tmp_crc = CRC_check((unsigned char *)&tmp_mp, sizeof(tmp_mp));
		rt_kprintf("mp crc %x , tmp_crc %x\r\n", crc,tmp_crc);
		if (length != sizeof(tmp_mp) || tmp_crc != crc)
		{
			rt_kprintf("check: mp crc not same, read mp data failed\n");
			close(fd);
			return 0;
		}
		memcpy(&mp,&tmp_mp,sizeof(mp));
	}
	close(fd);
	dump_mp(mp);
	
	fd = open(FQP_FILE, O_RDONLY, 0);
	if (fd > 0)
	{
		rt_kprintf("read fqp data\r\n");
		read(fd, &crc, sizeof(rt_uint16_t));
		length = read(fd, &tmp_fqp, sizeof(tmp_fqp));
		tmp_crc = CRC_check((unsigned char *)&tmp_fqp, sizeof(tmp_fqp));
		rt_kprintf("fqp crc %x , tmp_crc %x\r\n", crc,tmp_crc);
		if (length != sizeof(fqp)|| tmp_crc!=crc)
		{
			rt_kprintf("check: fqp crc not same, read fqp data failed\n");
			close(fd);
			return 0;
		}
		memcpy(&fqp,&tmp_fqp,sizeof(fqp));
		
		tmp_fangquList = (struct FangQu *)rt_malloc(WIRELESS_MAX*sizeof(struct FangQu));
		read(fd, &crc, sizeof(rt_uint16_t));
		length = read(fd, tmp_fangquList, sizeof(struct FangQu)*WIRELESS_MAX);
		tmp_crc = CRC_check((unsigned char *)tmp_fangquList, sizeof(struct FangQu)*WIRELESS_MAX);
		rt_kprintf("wireless crc %x , tmp_crc %x\r\n", crc,tmp_crc);
		if (length != sizeof(struct FangQu)*WIRELESS_MAX|| tmp_crc!=crc)
		{
			rt_kprintf("check: wireless crc not same, read failed\n");
			close(fd);
			rt_free(tmp_fangquList);
			return 0;
		}		
		memcpy(fangqu_wireless,tmp_fangquList,sizeof(struct FangQu)*WIRELESS_MAX);
		
		read(fd, &crc, sizeof(rt_uint16_t));
		length = read(fd, tmp_fangquList, sizeof(struct FangQu)*WIRE_MAX);
		tmp_crc = CRC_check((unsigned char *)tmp_fangquList, sizeof(struct FangQu)*WIRE_MAX);
		rt_kprintf("wire crc %x , tmp_crc %x\r\n", crc,tmp_crc);
		if (length != sizeof(struct FangQu)*WIRE_MAX|| tmp_crc!=crc)
		{
			rt_kprintf("check: wireless crc not same, read failed\n");
			close(fd);
			rt_free(tmp_fangquList);
			return 0;
		}
		memcpy(fangqu_wire,tmp_fangquList,sizeof(struct FangQu)*WIRE_MAX);

		rt_free(tmp_fangquList);
		close(fd);
	}

	fqp.delay_in=20;
	fqp.alarm_voice_time=30;
	fqp.is_alarm_voice =1;
	fqp.is_lamp = 0x04;
	mp.socketAddress[0].port = 1704;
	dump_fqp(fqp,fangqu_wire,fangqu_wireless);
	return 1;
}

void save_param(int type)
{
	int fd;
	int length;
	rt_uint16_t crc;
	rt_mutex_take(&file_lock,RT_WAITING_FOREVER);
	rt_kprintf("save_param %d\r\n",type);
	if (type == TYPE_MP)
	{
		fd = open(MP_FILE, O_WRONLY | O_CREAT | O_TRUNC, 0);
		crc = CRC_check((unsigned char *)&mp, sizeof(mp));
		rt_kprintf("crc %x\r\n", crc);
		length = write(fd, &crc, sizeof(rt_uint16_t));
		if (length != sizeof(rt_uint16_t))
		{
			rt_kprintf("write mp crc data failed %d\n",length);
		}
		length = write(fd, &mp, sizeof(mp));
		if (length != sizeof(mp))
		{
			rt_kprintf("write mp data failed %d\n",length);
			close(fd);
			rt_mutex_release(&file_lock);
			return ;
		}
		dump_mp(mp);
	}
	else
	{
		fd = open(FQP_FILE, O_WRONLY | O_CREAT | O_TRUNC, 0);
		crc = CRC_check((unsigned char *)&fqp, sizeof(struct FangQuProperty));
		rt_kprintf("fqp crc %x\r\n", crc);
		write(fd, &crc, sizeof(rt_uint16_t));
		length = write(fd, &fqp, sizeof(fqp));
		if (length != sizeof(fqp))
		{
			rt_kprintf("write fqp data failed %d\n",length);
			close(fd);
			rt_mutex_release(&file_lock);
			return ;
		}
		crc = CRC_check((unsigned char *)fangqu_wireless, sizeof(struct FangQu)*WIRELESS_MAX);
		rt_kprintf("wireless crc %x\r\n", crc);
		write(fd, &crc, sizeof(rt_uint16_t));
		length = write(fd, fangqu_wireless, sizeof(struct FangQu)*WIRELESS_MAX);
		if (length != sizeof(struct FangQu)*WIRELESS_MAX)
		{
			rt_kprintf("write wireless fq data failed %d\n",length);
			close(fd);
			rt_mutex_release(&file_lock);
			return ;
		}
		crc = CRC_check((unsigned char *)fangqu_wire, sizeof(struct FangQu)*WIRE_MAX);
		rt_kprintf("wire crc %x\r\n", crc);
		write(fd, &crc, sizeof(rt_uint16_t));
		length = write(fd, fangqu_wire, sizeof(struct FangQu)*WIRE_MAX);
		if (length != sizeof(struct FangQu)*WIRE_MAX)
		{
			rt_kprintf("write wire fq data failed %d\n",length);
			close(fd);
			rt_mutex_release(&file_lock);
			return ;
		}
		dump_fqp(fqp,fangqu_wire,fangqu_wireless);		
	}
	close(fd);
	rt_mutex_release(&file_lock);
	return;
}

