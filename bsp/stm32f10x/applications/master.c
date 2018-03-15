#include <board.h>
#include <rtthread.h>
#include <rtdevice.h>
#include <dfs_posix.h>
#include "master.h"
#include "lcd.h"
#include "led.h"
#include "bsp_misc.h"
#include "wtn6.h"
#define FQP_FILE	"/fqp.dat"
#define MP_FILE		"/mp.dat"
#define MAIN_STATION_PROTECT_ON 1
#define MAIN_STATION_PROTECT_OFF 0
#define DEFAULT_DOMAIN "kjfslkjflskdjfj"
struct rt_event g_info_event;
extern rt_uint8_t cur_status;
rt_uint32_t g_coding_cnt=0;
extern rt_uint8_t g_main_state;
extern rt_uint8_t in_fq;
rt_uint8_t g_num=0;
rt_uint8_t g_alarm_voice=0;
rt_uint8_t g_delay_in=0;
extern rt_uint8_t g_index_sub;
void dump_mp(struct MachineProperty v)
{
	int i;
	rt_kprintf("\r\n\r\nMainStation Param ......\r\n");
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
	rt_kprintf("\r\n\r\nFangQu Param ......\r\n");
	rt_kprintf("alarm_voice %d\r\n",v1.alarm_voice);
	rt_kprintf("delay_in %d\r\n",v1.delay_in);
	rt_kprintf("delya_out %d\r\n",v1.delya_out);
	rt_kprintf("delay_bufang %d\r\n",v1.delay_bufang);
	rt_kprintf("auto_bufang %x%x%x\r\n",v1.auto_bufang[0],
		v1.auto_bufang[1],
		v1.auto_bufang[2]);
	rt_kprintf("auto_chefang %x%x%x\r\n",v1.auto_chefang[0],
		v1.auto_chefang[1],
		v1.auto_chefang[2]);
	rt_kprintf("is_alarm_voice %d\r\n",v1.is_alarm_voice);
	rt_kprintf("is_check_AC %d\r\n",v1.is_check_AC);
	rt_kprintf("is_check_DC %d\r\n",v1.is_check_DC);
	rt_kprintf("PGM %d\r\n",v1.PGM);
	rt_kprintf("is_lamp %d\r\n",v1.is_lamp);
	for(i=0;i<WIRE_MAX;i++)
	{
		if(v2[i].index != 0) {
			rt_kprintf("FangQu[%d]\r\n",
				v2[i].index);
			rt_kprintf("type %d\r\n",v2[i].type);
			rt_kprintf("operationType %d\r\n",v2[i].operationType);
			rt_kprintf("voiceType %d\r\n",v2[i].voiceType);
			rt_kprintf("alarmType %d\r\n",v2[i].alarmType);
			rt_kprintf("isBypass %d\r\n",v2[i].isBypass);
			rt_kprintf("status %d\r\n",v2[i].status);
			rt_kprintf("slave_sn %08x\r\n",v2[i].slave_sn);
			rt_kprintf("slave_type %d\r\n",v2[i].slave_type);
			rt_kprintf("slave_model %d\r\n",v2[i].slave_model);
			rt_kprintf("slave_batch %d\r\n",v2[i].slave_batch);
		}
	}
	
	for(i=0;i<WIRELESS_MAX;i++)
	{
		if(v3[i].index != 0) {
			rt_kprintf("FangQu[%d]\r\n",
				v3[i].index);
			rt_kprintf("type %d\r\n",v3[i].type);
			rt_kprintf("operationType %d\r\n",v3[i].operationType);
			rt_kprintf("voiceType %d\r\n",v3[i].voiceType);
			rt_kprintf("alarmType %d\r\n",v3[i].alarmType);
			rt_kprintf("isBypass %d\r\n",v3[i].isBypass);
			rt_kprintf("status %d\r\n",v3[i].status);
			rt_kprintf("slave_sn %08x\r\n",v3[i].slave_sn);
			rt_kprintf("slave_type %d\r\n",v3[i].slave_type);
			rt_kprintf("slave_model %d\r\n",v3[i].slave_model);
			rt_kprintf("slave_batch %d\r\n",v3[i].slave_batch);
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
	//fangquList = (struct FangQu *)rt_malloc(140*sizeof(struct FangQu));
	rt_memset(&fqp, 0, sizeof(fqp));
	rt_memset(fangqu_wire, 0, sizeof(struct FangQu)*WIRE_MAX);
	rt_memset(fangqu_wireless, 0, sizeof(struct FangQu)*WIRELESS_MAX);
	dump_fqp(fqp,fangqu_wire,fangqu_wireless);
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

	fqp.alarm_voice=0;
	//fqp.auto_bufang=0;
	//fqp.auto_chefang=0;
	fqp.delay_bufang=0;
	fqp.delay_in=0;
	fqp.delya_out=0;
	fqp.is_alarm_voice=0;
	fqp.is_check_AC=0;
	fqp.is_check_DC=0;
	fqp.is_lamp=1;
	fqp.PGM=0;
	//int fd = -1;
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
		dump_mp(tmp_mp);
		tmp_crc = CRC_check((unsigned char *)&tmp_mp, sizeof(tmp_mp));
		rt_kprintf("crc %x , tmp_crc %x\r\n", crc,tmp_crc);
		if (length != sizeof(tmp_mp) || tmp_crc != crc)
		{
			rt_kprintf("check: read mp data failed\n");
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
		rt_kprintf("crc %x , tmp_crc %x\r\n", crc,tmp_crc);
		if (length != sizeof(fqp)|| tmp_crc!=crc)
		{
			rt_kprintf("check: read fqp data failed\n");
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
			rt_kprintf("check: wireless read fq data failed\n");
			close(fd);
			rt_free(tmp_fangquList);
			return 0;
		}
		
		//dump_fqp(tmp_fqp,tmp_fangquList);
		memcpy(fangqu_wireless,tmp_fangquList,sizeof(struct FangQu)*WIRELESS_MAX);
		
		read(fd, &crc, sizeof(rt_uint16_t));
		length = read(fd, tmp_fangquList, sizeof(struct FangQu)*WIRE_MAX);
		tmp_crc = CRC_check((unsigned char *)tmp_fangquList, sizeof(struct FangQu)*WIRE_MAX);
		rt_kprintf("wire crc %x , tmp_crc %x\r\n", crc,tmp_crc);
		if (length != sizeof(struct FangQu)*WIRE_MAX|| tmp_crc!=crc)
		{
			rt_kprintf("check: wire read fq data failed\n");
			close(fd);
			rt_free(tmp_fangquList);
			return 0;
		}
		
		//dump_fqp(tmp_fqp,tmp_fangquList);
		memcpy(fangqu_wire,tmp_fangquList,sizeof(struct FangQu)*WIRE_MAX);
		rt_free(tmp_fangquList);
		close(fd);
	}
	fqp.is_lamp=1;
	fqp.delay_in=20;
	fqp.alarm_voice=30;
	fqp.is_alarm_voice =1;
	dump_fqp(fqp,fangqu_wire,fangqu_wireless);
	return 1;
}
void save_param(int type)
{
	int fd;
	int length;
	rt_uint16_t crc;
	rt_kprintf("save_param %d\r\n",type);
	if (type == 0)
	{
		fd = open(MP_FILE, O_WRONLY | O_CREAT | O_TRUNC, 0);
		crc = CRC_check((unsigned char *)&mp, sizeof(mp));
		rt_kprintf("crc %x\r\n", crc);
		length = write(fd, &crc, sizeof(rt_uint16_t));
		if (length != sizeof(rt_uint16_t))
		{
			rt_kprintf("write crc data failed %d\n",length);
			//close(fd);
			//return ;
		}
		length = write(fd, &mp, sizeof(mp));
		if (length != sizeof(mp))
		{
			rt_kprintf("write mp data failed %d\n",length);
			close(fd);
			return ;
		}
		dump_mp(mp);
	}
	else
	{
		fd = open(FQP_FILE, O_WRONLY | O_CREAT | O_TRUNC, 0);
		crc = CRC_check((unsigned char *)&fqp, sizeof(struct FangQuProperty));
		rt_kprintf("crc %x\r\n", crc);
		write(fd, &crc, sizeof(rt_uint16_t));
		length = write(fd, &fqp, sizeof(fqp));
		if (length != sizeof(fqp))
		{
			rt_kprintf("write mp data failed %d\n",length);
			close(fd);
			return ;
		}
		crc = CRC_check((unsigned char *)fangqu_wireless, sizeof(struct FangQu)*WIRELESS_MAX);
		rt_kprintf("crc %x\r\n", crc);
		write(fd, &crc, sizeof(rt_uint16_t));
		length = write(fd, fangqu_wireless, sizeof(struct FangQu)*WIRELESS_MAX);
		if (length != sizeof(struct FangQu)*WIRELESS_MAX)
		{
			rt_kprintf("write wireless fq data failed %d\n",length);
			close(fd);
			return ;
		}
		crc = CRC_check((unsigned char *)fangqu_wire, sizeof(struct FangQu)*WIRE_MAX);
		rt_kprintf("crc %x\r\n", crc);
		write(fd, &crc, sizeof(rt_uint16_t));
		length = write(fd, fangqu_wire, sizeof(struct FangQu)*WIRE_MAX);
		if (length != sizeof(struct FangQu)*WIRE_MAX)
		{
			rt_kprintf("write wire fq data failed %d\n",length);
			close(fd);
			return ;
		}
		dump_fqp(fqp,fangqu_wire,fangqu_wireless);		
	}
	close(fd);
	return;
}
void info_user(void *param)
{
	rt_uint32_t ev;
	rt_uint8_t v;
	while (1) {
		rt_event_recv( &(g_info_event), 0xffffffff, RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR, RT_WAITING_FOREVER, &ev ); 
		if (ev & INFO_EVENT_CODING) {
			if (cur_status == 1) {
				/*need protection off first
				  *play audio here
				  */
				Wtn6_Play(VOICE_DUIMASB,ONCE);
				g_main_state = 0;
			} else {
				//SetErrorCode(0x01);
				//led_blink(1);
				Wtn6_Play(VOICE_DUIMAMS,ONCE);
				g_coding_cnt = 0;
			}
		}
		if (ev & INFO_EVENT_NORMAL) {
			SetErrorCode(0x00);
			rt_hw_led_off(0);
			Wtn6_Play(VOICE_TUICHUDM,ONCE);
		}
		if (ev & INFO_EVENT_FACTORY_RESET) {
			//SetErrorCode(0x02);
			led_blink(3);
			dfs_mkfs("elm","sd0");
			//__set_FAULTMASK();
			Wtn6_Play(VOICE_HUIFU,ONCE);
			NVIC_SystemReset();
		}
		if (ev & INFO_EVENT_PROTECT_ON) {
			SetStateIco(0,1);
			SetStateIco(1,0);
			rt_hw_led_on(ARM_LED);			
			v = fqp.is_lamp & 0x0f;
			Wtn6_Play(VOICE_BUFANG,ONCE);
			rt_kprintf("protect on is_lamp %x\r\n", fqp.is_lamp);
			if (v == 0x00)
			{
				GPIO_ResetBits(GPIOB, GPIO_Pin_7);
			} else if (v == 0x04 || v == 0x01 || v == 0x02) {
				GPIO_SetBits(GPIOB, GPIO_Pin_7);
			}
			if (fqp.is_alarm_voice)
			{
				GPIO_SetBits(GPIOC, GPIO_Pin_13);
				rt_thread_delay(100);
				GPIO_ResetBits(GPIOC, GPIO_Pin_13);
			}
		}
		if (ev & INFO_EVENT_PROTECT_OFF) {
			SetStateIco(1,1);
			SetStateIco(0,0);
			SetStateIco(2,0);
			rt_hw_led_off(ARM_LED);	
			rt_hw_led_off(ALARM_LED);	
			Wtn6_Play(VOICE_CHEFANG,ONCE);
			rt_kprintf("protect off is_lamp %x\r\n", fqp.is_lamp);
			v = fqp.is_lamp & 0x0f;
			if (v == 0x00 || v == 0x02 ||v == 0x03)
			{
				GPIO_ResetBits(GPIOB, GPIO_Pin_7);
			} else if (v == 0x04) {
				GPIO_SetBits(GPIOB, GPIO_Pin_7);
			}
			if (fqp.is_alarm_voice)
			{
				GPIO_SetBits(GPIOC, GPIO_Pin_13);
				rt_thread_delay(100);
				GPIO_ResetBits(GPIOC, GPIO_Pin_13);
			}
		
		}
		if (ev & INFO_EVENT_ALARM) {
			SetStateIco(2,1);
			rt_hw_led_on(ALARM_LED);
			rt_kprintf("is_lamp %d, is_alarm_voice %d, delay_in %d, alarm_voice %d\r\n",
				fqp.is_lamp,fqp.is_alarm_voice,fqp.delay_in,fqp.alarm_voice);
				if (fqp.is_alarm_voice)
					bell_ctl(1);
				if ((fangqu_wireless[g_index_sub].operationType==1) && fqp.delay_in && g_delay_in == 0) {
					g_alarm_voice = fqp.alarm_voice;
					g_delay_in = fqp.delay_in;
					Wtn6_Play(VOICE_ALARM2,LOOP);
				}
		}
		if (ev & INFO_EVENT_SHOW_NUM) {
			SetErrorCode(g_num);
		}
		if (ev & INFO_EVENT_SAVE_FANGQU) {
			Wtn6_Play(VOICE_DUIMA,ONCE);
			save_param(1);
		}
		if (ev & INFO_EVENT_SAVE_MAIN) {
			save_param(0);
		}
		if (ev & INFO_EVENT_MUTE) {
			rt_kprintf("do mute\r\n");
			Stop_Playing();
			bell_ctl(0);
		}
	}
}
void handle_login_ack(rt_uint8_t *cmd)
{
	rt_kprintf("login ack\r\n");
	rt_kprintf("status %x\r\n",cmd[0]);
	rt_kprintf("\r\nServer Time: %x%x%x%x%x%x%x\r\n",
	cmd[1],cmd[2],cmd[3],
	cmd[4],cmd[5],cmd[6],
	cmd[7]);
	rt_kprintf("len %d\r\n",cmd[8]);
	if (cmd[8] != 0) {
		rt_kprintf("new IP: ");
		for (int i=9;i<cmd[8]+9;i++)
			rt_kprintf("%c",cmd[i]);
	}
}
void handle_heart_beat_ack(rt_uint8_t *cmd)
{
}
void handle_t_logout_ack(rt_uint8_t *cmd)
{
}
void handle_alarm_trap_ack(rt_uint8_t *cmd)
{
}
void handle_get_address_ack(rt_uint8_t *cmd)
{
}

rt_uint8_t handle_packet(rt_uint8_t *data)
{
	int i=0;
	rt_uint8_t water_no = data[0];
	rt_uint16_t packet_type = (data[1]<<8)|data[2];
	rt_uint16_t protocl_v = (data[3]<<8)|data[4];
	rt_uint8_t stm32_id[6];
	memcpy(stm32_id, data+5,6);
	rt_kprintf("water no %d\r\n", water_no);
	rt_kprintf("pacet type %x\r\n", packet_type);
	rt_kprintf("protol version %d\r\n", protocl_v);
	rt_kprintf("sn %02x%02x%02x%02x%02x%02x\r\n",
		stm32_id[0],stm32_id[1],stm32_id[2],
		stm32_id[3],stm32_id[4],stm32_id[5]);
	if (memcmp(stm32_id,mp.roProperty.sn,6) != 0)
	{
		rt_kprintf("packet not for us %02x%02x%02x%02x%02x%02x\r\n",
			mp.roProperty.sn[0],mp.roProperty.sn[1],
			mp.roProperty.sn[2],mp.roProperty.sn[3],
			mp.roProperty.sn[4],mp.roProperty.sn[5]);
		return 0;
	}
	switch (packet_type)
	{
		case LOGIN_ACK:
			handle_login_ack(data+11);
			break;
		case HEART_BEAT_ACK:
			handle_heart_beat_ack(data+11);
			break;
		case T_LOGOUT_ACK:
			handle_t_logout_ack(data+11);
			break;
		case ALARM_TRAP_ACK:
			handle_alarm_trap_ack(data+11);
			break;
		case GET_ADDRESS_ACK:
			handle_get_address_ack(data+11);
			break;
		default:
			rt_kprintf("unknown packet type\r\n");
			break;
	}
	return 1;
}
