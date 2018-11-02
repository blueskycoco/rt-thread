#include <board.h>
#include <rtthread.h>
#include <rtdevice.h>
#include <rthw.h>
#include <dfs_posix.h>
#include "prop.h"
#include "lcd.h"
#include "led.h"
#include "bsp_misc.h"
#include "wtn6.h"
#include "subpoint.h"
#include "pcie.h"
rt_uint32_t 	g_fangqu_ts_cnt = 0;
extern rt_uint8_t g_alarm_fq;
struct rt_mutex file_lock;
extern rt_uint16_t g_alarm_reason;
void dump_mp(struct MachineProperty v)
{
	int i;
	rt_kprintf("\r\n\r\n<<MainStation Param>>\r\n");
	rt_kprintf("reload %d\r\n",v.reload);
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
	rt_kprintf("hdVersion \t\t%d\r\n", hwv.hdVersion);
	rt_kprintf("isdVersion \t\t%d\r\n", hwv.isdVersion);
	rt_kprintf("lcddVersion \t\t%d\r\n\r\n", hwv.lcdVersion);
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
			rt_kprintf("isStay \t\t%d\r\n",v2[i].isStay);
			rt_kprintf("status \t\t%d\r\n",v2[i].status);
			rt_kprintf("slave_delay \t%d\r\n",v2[i].slave_delay);
			rt_kprintf("slave_type \t%x\r\n",v2[i].slave_type);
			rt_kprintf("slave_model \t%x\r\n",v2[i].slave_model);
			rt_kprintf("slave_batch \t%x\r\n",v2[i].slave_batch);
			rt_kprintf("slave_sn \t%08x\r\n",v2[i].slave_sn);
			if (v2[i].alarmType == TYPE_24)
				v2[i].status = TYPE_PROTECT_ON;
		}
	}

	rt_kprintf("\r\nwireless fq\r\n");
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
			rt_kprintf("isStay \t\t%d\r\n",v3[i].isStay);
			rt_kprintf("status \t\t%d\r\n",v3[i].status);
			rt_kprintf("slave_delay \t%d\r\n",v3[i].slave_delay);
			rt_kprintf("slave_type \t%x\r\n",v3[i].slave_type);
			rt_kprintf("slave_model \t%x\r\n",v3[i].slave_model);
			rt_kprintf("slave_batch \t%x\r\n",v3[i].slave_batch);
			rt_kprintf("slave_sn \t%08x\r\n",v3[i].slave_sn);			
			if (v3[i].alarmType == TYPE_24)
				v3[i].status = TYPE_PROTECT_ON;
		}
	}
}
void default_fqp_t(struct FangQu *v2,struct FangQu *v3)
{
	int i;
	for(i=0;i<WIRE_MAX;i++)
	{
		if(v2[i].index != 0) {
			fangqu_ts[g_fangqu_ts_cnt].new_code = 0;
			fangqu_ts[g_fangqu_ts_cnt].heart_ts = 0;
			fangqu_ts[g_fangqu_ts_cnt].off_line = 0;
			fangqu_ts[g_fangqu_ts_cnt].off_line2 = 0;
			fangqu_ts[g_fangqu_ts_cnt].index = v2[i].index;
			fangqu_ts[g_fangqu_ts_cnt].slave_type = v2[i].slave_type;
			rt_kprintf("ts %d , index %d\r\n", g_fangqu_ts_cnt, v2[i].index);
			g_fangqu_ts_cnt++;
		}
	}
	for(i=0;i<WIRELESS_MAX;i++)
	{
		if(v3[i].index != 0) {
			fangqu_ts[g_fangqu_ts_cnt].new_code = 0;
			fangqu_ts[g_fangqu_ts_cnt].heart_ts = 0;
			fangqu_ts[g_fangqu_ts_cnt].off_line = 0;
			fangqu_ts[g_fangqu_ts_cnt].off_line2 = 0;
			fangqu_ts[g_fangqu_ts_cnt].index = v3[i].index;				
			fangqu_ts[g_fangqu_ts_cnt].slave_type = v3[i].slave_type;
			rt_kprintf("ts %d , index %d\r\n", g_fangqu_ts_cnt, v3[i].index);
			g_fangqu_ts_cnt++;
		}
	}
}
void default_fqp_t2()
{
	int i;
	rt_time_t ts = time(RT_NULL);
	for(i=0;i<g_fangqu_ts_cnt;i++)
	{
		//if (fangqu_ts[i].heart_ts == 0)
		fangqu_ts[i].heart_ts = ts;
	}
}
void add_fqp_t(rt_uint8_t index, rt_uint8_t slave_type)
{		
	int i;
	for (i=0; i<g_fangqu_ts_cnt; i++) {
		if (fangqu_ts[i].index == index)
			break;
	}
	if (i==g_fangqu_ts_cnt){
		rt_kprintf("add fqp t %d\r\n", index);	
		rt_uint32_t fqt = time(RT_NULL);
		if (fqt > 1000)
			fangqu_ts[g_fangqu_ts_cnt].heart_ts = time(RT_NULL);
		else
			fangqu_ts[g_fangqu_ts_cnt].heart_ts = 0;
		if (slave_type == 0x42) {
			fangqu_ts[g_fangqu_ts_cnt].new_code = 1;
			fangqu_ts[g_fangqu_ts_cnt].new_code_ts = fangqu_ts[g_fangqu_ts_cnt].heart_ts;
		}
		fangqu_ts[g_fangqu_ts_cnt].off_line = 0;
		fangqu_ts[g_fangqu_ts_cnt].off_line2 = 0;
		fangqu_ts[g_fangqu_ts_cnt].index = index;
		fangqu_ts[g_fangqu_ts_cnt].slave_type = slave_type;
		g_alarm_fq = fangqu_ts[g_fangqu_ts_cnt].index;
		g_alarm_reason = 0x0017;
		g_fangqu_ts_cnt++;
		upload_server(0x0004); 		
		
		for (i=0; i<g_fangqu_ts_cnt; i++)
			rt_kprintf("after add fangqu ts %d %x %d\r\n",
			fangqu_ts[i].index,fangqu_ts[i].slave_type,fangqu_ts[i].heart_ts);
	}
}
void del_fqp_t(rt_uint8_t index)
{
	int i;
	rt_kprintf("del fqp t %d\r\n", index);
	for (i=0; i<g_fangqu_ts_cnt; i++) {
		if (fangqu_ts[i].index == index)
			break;
	}
	rt_kprintf("i %d %d\r\n", i,g_fangqu_ts_cnt);
	if (i == g_fangqu_ts_cnt)
		return;
	else if (i == (g_fangqu_ts_cnt - 1)) {
		g_fangqu_ts_cnt--;
		rt_memset(fangqu_ts+g_fangqu_ts_cnt,0,sizeof(struct FangQuT));
	} else {
		for (;i<g_fangqu_ts_cnt-1; i++) {
			memcpy(fangqu_ts+i,fangqu_ts+i+1,sizeof(struct FangQuT));	
		}
		g_fangqu_ts_cnt--;
	}
	for (i=0; i<g_fangqu_ts_cnt; i++)
		rt_kprintf("after delete fangqu ts %d %x %d\r\n",
		fangqu_ts[i].index,fangqu_ts[i].slave_type,fangqu_ts[i].heart_ts);
}
void record_fqp_ts(rt_uint8_t index)
{
	int i;	
	rt_time_t cur_time = time(RT_NULL);
	for (i=0; i<g_fangqu_ts_cnt; i++) {
		if (fangqu_ts[i].index == index) {
			fangqu_ts[i].heart_ts = cur_time;
			fangqu_ts[i].off_line2 = 0;
		}
	}
}
rt_uint8_t fangqu_offline(rt_uint8_t index)
{
	int i;
	for (i=0; i<g_fangqu_ts_cnt; i++) {
		if (fangqu_ts[i].index == index)
			return !(fangqu_ts[i].off_line2);
	}
	return 0x00;
}
void check_off_line_alarm()
{
	int i;	
	int timeout_ts = 600;
	rt_time_t cur_time = time(RT_NULL);
	for (i=0; i<g_fangqu_ts_cnt; i++) {
		rt_kprintf("fq[%d]\t %d %d %d, cur %d, %d\t%s\r\n", fangqu_ts[i].index,fangqu_ts[i].heart_ts,
			fangqu_ts[i].off_line,fangqu_ts[i].off_line2,cur_time,cur_time - fangqu_ts[i].heart_ts, cmd_dev_type(fangqu_ts[i].slave_type));
//		if (fangqu_ts[i].new_code && ((cur_time - fangqu_ts[i].new_code_ts) > 21600) ) {
//			rt_kprintf("reset fangqu %d, infrar to delay mode\r\n", fangqu_ts[i].index-2);
//			fangqu_wireless[fangqu_ts[i].index-2].slave_delay = TYPE_SLAVE_MODE_DELAY;
//			fangqu_ts[i].new_code=0;
//		}
		if (fangqu_ts[i].heart_ts != 0 && fangqu_ts[i].off_line == 0) {
			if (fangqu_ts[i].index < 50) 
				timeout_ts = 25*60*60;
			
			if ((cur_time - fangqu_ts[i].heart_ts) > timeout_ts) {
				if (fangqu_ts[i].slave_type != 0x01 && fangqu_ts[i].off_line == 0) {
					g_alarm_fq = fangqu_ts[i].index;
					g_alarm_reason = 0x0005;
					upload_server(0x0004);
				}
				fangqu_ts[i].heart_ts = cur_time;
				fangqu_ts[i].off_line = 1;
				fangqu_ts[i].off_line2 = 1;
				break;
			}
		}
		if (fangqu_ts[i].off_line == 1 && fangqu_ts[i].off_line2 == 0) {			
			if (fangqu_ts[i].slave_type != 0x01) {
			g_alarm_fq = fangqu_ts[i].index;
			g_alarm_reason = 0x0017;
			upload_server(0x0004);
			fangqu_ts[i].off_line = 0;
			}
			break;
		}
	}
}
void upload_sub_status()
{	
	int i;
	for (i=0; i<g_fangqu_ts_cnt; i++) {
		if (fangqu_ts[i].off_line2 == 1) {			
			if (fangqu_ts[i].slave_type != 0x01) {
				g_alarm_fq = fangqu_ts[i].index;
				g_alarm_reason = 0x0005;
				upload_server(0x0004);
				}
			}
	}
}
void default_fqp()
{
	int i;
	
	fqp.delya_out=30;
	fqp.delay_in=0;
	fqp.alarm_voice_time=1;
	fqp.is_alarm_voice =1;
	fqp.is_lamp = 0x01;
	fqp.status=0;
	fqp.auto_bufang=0xffffffff;
	fqp.auto_chefang=0xffffffff;
	fqp.PGM0=0;
	fqp.PGM1=0;
	fqp.is_check_AC=1;
	fqp.is_check_DC=1;
	for(i=0;i<WIRE_MAX;i++)
	{
		if(fangqu_wire[i].index != 0) {
			fangqu_wire[i].voiceType =TYPE_VOICE_Y;
			fangqu_wire[i].operationType= TYPE_DELAY;
			fangqu_wire[i].alarmType= TYPE_ALARM_00;
			fangqu_wire[i].slave_delay = TYPE_SLAVE_MODE_DELAY;
			fangqu_wire[i].status= TYPE_PROTECT_OFF;
			fangqu_wire[i].isStay= TYPE_STAY_N;
			fangqu_wire[i].isBypass= TYPE_BYPASS_N;
		}
	}

	for(i=0;i<WIRELESS_MAX;i++)
	{
		if(fangqu_wireless[i].index != 0) {
			fangqu_wireless[i].voiceType =TYPE_VOICE_Y;
			fangqu_wireless[i].operationType= TYPE_DELAY;
			fangqu_wireless[i].alarmType= TYPE_ALARM_00;
			fangqu_wireless[i].slave_delay = TYPE_SLAVE_MODE_DELAY;
			fangqu_wireless[i].status= TYPE_PROTECT_OFF;
			fangqu_wireless[i].isStay= TYPE_STAY_N;
			fangqu_wireless[i].isBypass= TYPE_BYPASS_N;
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
	struct HwVersion	tmp_hwv;
	struct FangQu *tmp_fangquList;
	rt_mutex_init(&file_lock,	"file_lock",	RT_IPC_FLAG_FIFO);

	rt_memset(&fqp, 0, sizeof(fqp));
	rt_memset(fangqu_wire, 0, sizeof(struct FangQu)*WIRE_MAX);
	rt_memset(fangqu_wireless, 0, sizeof(struct FangQu)*WIRELESS_MAX);
	rt_memset(&mp, 0, sizeof(struct MachineProperty));
	//list_dir("/");
	mp.reload = 0;
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

	fqp.alarm_voice_time=1;
	fqp.audio_vol = 8;
	fqp.auto_bufang=0xffff;
	fqp.auto_chefang=0xffff;
	fqp.delay_in=DEFAULT_DELAY_IN;
	fqp.delya_out=DEFAULT_DELAY_OUT;
	fqp.is_alarm_voice=DEFAULT_ALARM_VOICE;
	fqp.is_check_AC=DEFAULT_CHECK_AC;
	fqp.is_check_DC=DEFAULT_CHECK_DC;
	fqp.is_lamp = 0x04;
	fqp.PGM0=0;
	fqp.PGM1=1;
	fqp.status=0;
	default_fqp();

	int fd = open(MP_FILE, O_RDONLY, 0);
	if (fd < 0)
	{
		rt_kprintf("INIT factory\n");
		fd = open(MP_FILE, O_WRONLY | O_CREAT | O_TRUNC, 0);
		rt_uint16_t crc = CRC_check((unsigned char *)&mp, sizeof(mp));
		rt_kprintf("init crc %x\r\n", crc);
		write(fd, &crc, sizeof(rt_uint16_t));
		fsync(fd);
		length = write(fd, &mp, sizeof(mp));
		fsync(fd);
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
	fd = open(HWV_FILE, O_RDONLY, 0);
	if (fd > 0)
	{
		rt_kprintf("read hwv data\r\n");
		read(fd, &crc, sizeof(rt_uint16_t));
		length = read(fd, &tmp_hwv, sizeof(tmp_hwv));
		tmp_crc = CRC_check((unsigned char *)&tmp_hwv, sizeof(tmp_hwv));
		rt_kprintf("hwv crc %x , tmp_crc %x\r\n", crc,tmp_crc);
		if (length != sizeof(hwv)|| tmp_crc!=crc)
		{
			rt_kprintf("check: hwv crc not same, read hwv data failed\n");
			//close(fd);
			//return 0;
		} else
			memcpy(&hwv,&tmp_hwv,sizeof(hwv));
		close(fd);
	} 
	else
	{		
		hwv.hdVersion=30;
		hwv.lcdVersion=1;
		hwv.isdVersion=1;
	}
	/*mp.socketAddress[0].port = 8434;
	
	mp.socketAddress[0].IP[0] = 220;
	mp.socketAddress[0].IP[1] = 180;
	mp.socketAddress[0].IP[2] = 239;
	mp.socketAddress[0].IP[3] = 212;
	*/dump_fqp(fqp,fangqu_wire,fangqu_wireless);
	default_fqp_t(fangqu_wire,fangqu_wireless);
	return 1;
}

void save_param(int type)
{
	int fd;
	int length;
	rt_base_t level;
	rt_uint16_t crc;
	rt_mutex_take(&file_lock,RT_WAITING_FOREVER);
	rt_kprintf("save_param %d\r\n",type);
	if (type == TYPE_MP)
	{
		fd = open(MP_FILE, O_WRONLY | O_CREAT | O_TRUNC, 0);
		if (fd<0) {
			rt_mutex_release(&file_lock);
			rt_kprintf("save open failed 0\r\n");
			return ;
		}
		crc = CRC_check((unsigned char *)&mp, sizeof(mp));
		rt_kprintf("crc %x\r\n", crc);
		level = rt_hw_interrupt_disable();
		length = write(fd, &crc, sizeof(rt_uint16_t));
		//fsync(fd);
		//rt_hw_interrupt_enable(level);
		if (length != sizeof(rt_uint16_t))
		{
			rt_kprintf("write mp crc data failed %d\n",length);
		}
		//level = rt_hw_interrupt_disable();
		length = write(fd, &mp, sizeof(mp));
		fsync(fd);
		rt_hw_interrupt_enable(level);
		if (length != sizeof(mp))
		{
			rt_kprintf("write mp data failed %d\n",length);
			close(fd);
			rt_mutex_release(&file_lock);
			return ;
		}
		//dump_mp(mp);
	}
	else
	{
		fd = open(FQP_FILE, O_WRONLY | O_CREAT | O_TRUNC, 0);
		crc = CRC_check((unsigned char *)&fqp, sizeof(struct FangQuProperty));
		rt_kprintf("fqp crc %x\r\n", crc);
		level = rt_hw_interrupt_disable();
		write(fd, &crc, sizeof(rt_uint16_t));
		//fsync(fd);
		length = write(fd, &fqp, sizeof(fqp));
		fsync(fd);
		rt_hw_interrupt_enable(level);
		if (length != sizeof(fqp))
		{
			rt_kprintf("write fqp data failed %d\n",length);
			close(fd);
			rt_mutex_release(&file_lock);
			return ;
		}
		crc = CRC_check((unsigned char *)fangqu_wireless, sizeof(struct FangQu)*WIRELESS_MAX);
		rt_kprintf("wireless crc %x\r\n", crc);
		level = rt_hw_interrupt_disable();
		write(fd, &crc, sizeof(rt_uint16_t));
		//fsync(fd);
		length = write(fd, fangqu_wireless, sizeof(struct FangQu)*WIRELESS_MAX);
		fsync(fd);
		rt_hw_interrupt_enable(level);
		if (length != sizeof(struct FangQu)*WIRELESS_MAX)
		{
			rt_kprintf("write wireless fq data failed %d\n",length);
			close(fd);
			rt_mutex_release(&file_lock);
			return ;
		}
		crc = CRC_check((unsigned char *)fangqu_wire, sizeof(struct FangQu)*WIRE_MAX);
		rt_kprintf("wire crc %x\r\n", crc);
		level = rt_hw_interrupt_disable();
		write(fd, &crc, sizeof(rt_uint16_t));
		//fsync(fd);
		length = write(fd, fangqu_wire, sizeof(struct FangQu)*WIRE_MAX);
		fsync(fd);
		rt_hw_interrupt_enable(level);
		if (length != sizeof(struct FangQu)*WIRE_MAX)
		{
			rt_kprintf("write wire fq data failed %d\n",length);
			close(fd);
			rt_mutex_release(&file_lock);
			return ;
		}
		//dump_fqp(fqp,fangqu_wire,fangqu_wireless);		
	}
	close(fd);
	rt_mutex_release(&file_lock);
	return;
}

