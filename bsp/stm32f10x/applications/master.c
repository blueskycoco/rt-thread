#include <board.h>
#include <rtthread.h>
#include <rtdevice.h>
#include <dfs_posix.h>
#include "prop.h"
#include "lcd.h"
#include "led.h"
#include "bsp_misc.h"
#include "wtn6.h"
#include "master.h"
#include "button.h"
#include "subPoint.h"
struct rt_event g_info_event;
extern rt_uint8_t cur_status;
rt_uint8_t g_num=0;
rt_uint32_t g_coding_cnt=0;
rt_uint8_t g_delay_in=0;
rt_uint8_t g_delay_out=0;
extern rt_uint8_t g_index_sub;
rt_uint8_t g_alarm_voice=0;
extern rt_uint8_t g_main_state;
extern rt_uint8_t g_alarmType;
extern rt_uint8_t s1;
extern rt_uint8_t g_flag;
void handle_led(int type)
{
	rt_uint8_t v;
	if (type == TYPE_PROTECT_ON) {
		v = fqp.is_lamp & 0x0f;
		rt_kprintf("protect on is_lamp %x\r\n", fqp.is_lamp);
		if (v == 0x00)
		{
			GPIO_ResetBits(GPIOB, GPIO_Pin_7);
		} else if (v == 0x04 || v == 0x01 || v == 0x02) {
			GPIO_SetBits(GPIOB, GPIO_Pin_7);
			if (v==0x04)
			rt_hw_led_on(AUX_LED0);
		}

	} else if (type == TYPE_PROTECT_OFF) {
		rt_kprintf("protect off is_lamp %x\r\n", fqp.is_lamp);
		v = fqp.is_lamp & 0x0f;
		if (v == 0x00 || v == 0x02 ||v == 0x03)
		{
			GPIO_ResetBits(GPIOB, GPIO_Pin_7);
		} else if (v == 0x04) {
			GPIO_SetBits(GPIOB, GPIO_Pin_7);
			rt_hw_led_on(AUX_LED1);
		}

	} else {
	}
}
void info_user(void *param)
{
	rt_uint32_t ev;
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
				g_main_state = 1;		
				Wtn6_Play(VOICE_DUIMAMS,ONCE);
				g_coding_cnt = 0;
			}
		}
		if (ev & INFO_EVENT_NORMAL) {
			SetErrorCode(0x00);
			rt_hw_led_off(0);
			if (g_main_state == 1) {
			Wtn6_Play(VOICE_TUICHUDM,ONCE);
			g_main_state = 0;	
			}
		}
		if (ev & INFO_EVENT_FACTORY_RESET) {
			g_main_state = 2;
			led_blink(3);
			dfs_mkfs("elm","sd0");
			Wtn6_Play(VOICE_HUIFU,ONCE);
			NVIC_SystemReset();
		}
		if (ev & INFO_EVENT_PROTECT_ON) {
			SetStateIco(0,1);
			SetStateIco(1,0);
			rt_hw_led_on(ARM_LED);
			cur_status = 1;						
			Wtn6_Play(VOICE_BUFANG,ONCE);
			rt_kprintf("bufang ok\r\n");
			handle_led(TYPE_PROTECT_ON);
			
			if (fqp.is_alarm_voice)
			{
				GPIO_SetBits(GPIOC, GPIO_Pin_13);
				rt_thread_delay(100);
				GPIO_ResetBits(GPIOC, GPIO_Pin_13);
			}
			
			fqp.status=cur_status;
			set_fq_on(fangqu_wire,WIRE_MAX);
			set_fq_on(fangqu_wireless,WIRELESS_MAX);			
			rt_event_send(&(g_info_event), INFO_EVENT_SAVE_FANGQU);
		}		
		if (ev & INFO_EVENT_DELAY_PROTECT_ON) {
			Wtn6_Play(VOICE_YANSHIBF,LOOP);
			g_delay_out = fqp.delya_out;
			g_alarm_voice = fqp.alarm_voice_time;
			rt_kprintf("yanshi delay out %d, alarm voice %d\r\n",g_delay_out,g_alarm_voice);
		}
		if (ev & INFO_EVENT_PROTECT_OFF) {
			SetStateIco(1,1);
			SetStateIco(0,0);
			SetStateIco(2,0);
			rt_hw_led_off(ARM_LED);	
			rt_hw_led_off(ALARM_LED);	
			Wtn6_Play(VOICE_CHEFANG,ONCE);
			handle_led(TYPE_PROTECT_OFF);
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
			rt_kprintf("is_lamp %d, is_alarm_voice %d, delay_in %d, alarm_voice %d, voiceType %d\r\n",
				fqp.is_lamp,fqp.is_alarm_voice,fqp.delay_in,fqp.alarm_voice_time,fangqu_wireless[g_index_sub].voiceType);
			/*handle alarm voice*/
				if (s1==1) {
					if (fqp.is_alarm_voice) {
						bell_ctl(1);
						rt_thread_delay(100);
					}
				} else {

					if (fangqu_wireless[g_index_sub].voiceType == 0 && fqp.is_alarm_voice && fqp.alarm_voice_time>0)
					{
						if ( fangqu_wireless[g_index_sub].operationType==0 || g_alarmType == 2)
							g_alarm_voice = fqp.alarm_voice_time;
						
						if (fangqu_wireless[g_index_sub].operationType != 1)
							bell_ctl(1);
					}
				}
			/*handle audio*/
			if (fangqu_wireless[g_index_sub].voiceType == 0) {
					if (s1 == 0 && fangqu_wireless[g_index_sub].operationType !=2) { //non-emergency
						rt_kprintf("non-emergency audio play\r\n");
						if (fangqu_wireless[g_index_sub].operationType == 1 && fqp.delay_in > 0) { //delay mode
							//g_alarm_voice = fqp.alarm_voice_time;
							rt_kprintf("non-emergency audio delay mode\r\n");
							g_flag=0;
							g_delay_in = fqp.delay_in;
							Wtn6_Play(VOICE_ALARM2,LOOP);
						} else {
							rt_kprintf("non-emergency audio normal mode\r\n");
							g_alarm_voice = fqp.alarm_voice_time;
							Wtn6_Play(VOICE_ALARM2,LOOP);
						}
					} else {
						rt_kprintf("emergency audio play\r\n");
						if (s1 == 1) { //s1 switch
							rt_kprintf("s1 audio\r\n");
							Wtn6_Play(VOICE_CHEFANG,ONCE);
						} else {
							rt_kprintf("non-s1 audio \r\n");
							g_alarm_voice = fqp.alarm_voice_time;
							rt_uint8_t action = ONCE;
							if (g_alarm_voice >0)
								action = LOOP;
							else
								action = ONCE;
							if (g_alarmType == 0) {//normal		
								rt_kprintf("normal\r\n");
								Wtn6_Play(VOICE_ZHUJIGZ,action);
							} else if (g_alarmType == 1) {//fire
								rt_kprintf("fire\r\n");
								Wtn6_Play(VOICE_YANSHIBF,action);
							} else if (g_alarmType == 2) {//emergency
								rt_kprintf("emergency\r\n");
								Wtn6_Play(VOICE_TUICHUDM,action);
							} else if (g_alarmType == 3) {//medical
								rt_kprintf("medical\r\n");
								Wtn6_Play(VOICE_SHANCHU,action);
							} else if (g_alarmType == 4) {
								rt_kprintf("4 \r\n");
								Wtn6_Play(VOICE_JIAOLIUDD,action);
							}
						}
					}
			}
				/*
				if (s1 !=0 && g_alarmType !=2 && (fangqu_wireless[g_index_sub].operationType==1) && fqp.delay_in && g_delay_in == 0) {
					g_alarm_voice = fqp.alarm_voice_time;
					g_delay_in = fqp.delay_in;
					Wtn6_Play(VOICE_ALARM2,LOOP);
				}*/
		}
		
		if (ev & INFO_EVENT_SHOW_NUM) {
			SetErrorCode(g_num);
		}
		if (ev & INFO_EVENT_SAVE_FANGQU) {
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
