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
#include "pcie.h"
#include "can.h"
struct rt_event g_info_event;
extern rt_uint8_t cur_status;
extern rt_uint8_t 	g_mute;
rt_uint8_t g_num=0;
rt_uint32_t g_coding_cnt=0;
rt_uint8_t g_delay_in=0;
rt_uint8_t g_delay_out=0;
extern rt_uint8_t g_index_sub;
rt_uint8_t g_alarm_voice=0;
rt_uint8_t startup_flag = 0;
extern rt_uint8_t g_main_state;
extern rt_uint8_t g_alarmType;
extern rt_uint8_t s1;
extern rt_uint8_t g_flag;
extern rt_uint8_t g_net_state;
rt_uint8_t alarm_led = 0;
rt_uint16_t pgm0_cnt=0;
rt_uint16_t pgm1_cnt=0;
extern rt_uint8_t g_exit_reason;
extern rt_uint16_t g_alarm_reason;
extern rt_uint8_t g_alarm_fq;
extern rt_uint16_t g_main_event_code;
extern rt_uint8_t g_operate_platform;
extern rt_uint8_t g_operater[6];
extern rt_uint16_t g_sub_event_code;
extern rt_uint8_t g_fq_len;
extern rt_uint8_t g_fq_event[10];
extern rt_uint8_t g_addr_type;
extern rt_uint16_t command_type;
extern rt_uint8_t g_ac;
extern rt_uint8_t g_heart_cnt;
extern rt_uint8_t g_addr_type;
extern struct rt_mutex g_stm32_lock;
extern rt_uint8_t entering_ftp_mode;
rt_uint16_t g_app_v=0;
extern rt_uint8_t time_protect;
rt_uint8_t g_yanshi = 0;
rt_uint8_t g_remote_protect = 0;
extern rt_uint8_t duima_key;
rt_uint8_t g_fq_index;
rt_uint8_t  	g_operationType;
rt_uint8_t  	g_voiceType;
rt_uint16_t g_crc;
rt_uint8_t *g_ftp = RT_NULL;
extern rt_uint8_t s_bufang;
extern rt_uint8_t g_need_reset_rtc;
extern rt_uint8_t 	sub_cmd_type;
extern struct rt_semaphore cc1101_rx_sem;
extern rt_uint8_t wire_code;
extern int g_index;
extern rt_uint8_t g_type0;
extern rt_uint8_t g_type1;
extern rt_uint32_t g_nbiot_update_len;
rt_uint8_t nbiot_module = 0;
extern rt_uint8_t entering_ftp_mode_nbiot;
extern rt_uint8_t g_module_type;
rt_uint8_t g_wire_type;
rt_uint32_t g_wire_addr;
//rt_uint8_t net_flow_flag=0;
int bc28_down_fd = -1;
extern rt_uint8_t in_qiact;
extern rt_uint32_t qiact_times;
rt_uint8_t m26_restart_flag;
extern void set_alarm_now();
extern void reset_alarm_now();
extern 	void begin_yunduo();
rt_uint8_t upgrade_type = 0; /* 0 for Boot, 1 for App*/
rt_uint8_t g_all_fq_num = 0;
rt_uint8_t g_all_fq_num_bak = 0;
rt_uint16_t g_async_alarm = 0;
rt_uint16_t g_async_event = 0;
void handle_led(int type)
{
	rt_uint8_t v;
	if (type == TYPE_PROTECT_ON) {
		v = fqp.is_lamp & 0x0f;
		rt_kprintf("protect on is_lamp %x\r\n", fqp.is_lamp);
		if (v == 0x00)
		{
			//GPIO_ResetBits(GPIOB, GPIO_Pin_7);
		} else if (v == 0x04 || v == 0x01 || v == 0x02) {
			//GPIO_SetBits(GPIOB, GPIO_Pin_7);
			if (v==0x04 && g_ac)
				rt_hw_led_on(AUX_LED0);
		}

	} else if (type == TYPE_PROTECT_OFF) {
		rt_kprintf("protect off is_lamp %x\r\n", fqp.is_lamp);
		v = fqp.is_lamp & 0x0f;
		if (v == 0x00 || v == 0x02 ||v == 0x03)
		{
			//GPIO_ResetBits(GPIOB, GPIO_Pin_7);
		} else if (v == 0x04) {
			//GPIO_SetBits(GPIOB, GPIO_Pin_7);
			if (g_ac)
				rt_hw_led_on(AUX_LED1);
		}

	} else {
	}
}
void pgm_ctl(int type)
{
	if (type != 2 && type != 4) {
		if (fqp.PGM0 == type || fqp.PGM1 == type) {
			if (fqp.PGM0==type)
				rt_hw_led_on(PGM3_LED);
			if (fqp.PGM1==type)
				rt_hw_led_on(PGM4_LED);
			rt_thread_delay(100);
			if (fqp.PGM0==type)
				rt_hw_led_off(PGM3_LED);
			if (fqp.PGM1==type)
				rt_hw_led_off(PGM4_LED);
		} 
	}else {
		if (fqp.PGM0 == type) {
			rt_hw_led_on(PGM3_LED);
			pgm0_cnt = 300;
		}
		if (fqp.PGM1 == type) {
			rt_hw_led_on(PGM4_LED);
			pgm1_cnt = 300;
		}
	}
}
void pgm_ctl2(int type)
{
	if (0x01 == type) {
		if (fqp.PGM0 == 3) {
			rt_hw_led_on(PGM3_LED);
			rt_thread_delay(100);
			rt_hw_led_off(PGM3_LED);
		} else if (fqp.PGM0 == 4) {
			rt_hw_led_on(PGM3_LED);
			pgm0_cnt = 300;
		}
	} else if (0x02 == type) {
		if (fqp.PGM1 == 3) {
			rt_hw_led_on(PGM4_LED);
			rt_thread_delay(100);
			rt_hw_led_off(PGM4_LED);
		} else if (fqp.PGM1 == 4) {
			rt_hw_led_on(PGM4_LED);
			pgm1_cnt = 300;
		}
	}
}
void info_user(void *param)
{
	rt_uint32_t ev;
	while (1) {
		rt_event_recv( &(g_info_event), 0xffffffff, RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR, RT_WAITING_FOREVER, &ev ); 
		rt_mutex_take(&g_stm32_lock,RT_WAITING_FOREVER);
		rt_kprintf("get message %x\r\n", ev);
		if (ev & INFO_EVENT_CODING) {
			if (cur_status == 1) {
				/*need protection off first
				 *play audio here
				 */
				Wtn6_Play(VOICE_XIANCHEFANG,ONCE,1);
				g_main_state = 0;
			} else {
				//SetErrorCode(0x01);
				//led_blink(1);
				g_main_state = 1;		
				Wtn6_Play(VOICE_DUIMAMS,ONCE,1);
				g_coding_cnt = 0;
			}
		}
		if (ev & INFO_EVENT_NORMAL) {
			SetErrorCode(0x00);
			rt_hw_led_off(0);
			if (g_main_state == 1) {
				Wtn6_Play(VOICE_TUICHUDM,ONCE,1);
				g_main_state = 0;	
			}
			/*
			   if (startup_flag == 0) {
			   startup_flag = 1;
			   } else
			   upload_server(CMD_ASK_SUB_ACK);
			   */
		}
		if (ev & INFO_EVENT_FACTORY_RESET) {
			g_main_state = 2;
			Stop_Playing();
			led_blink(3);
			g_exit_reason = 0x03;
			upload_server(CMD_EXIT);
			//dfs_mkfs("elm","sd0");
			Wtn6_Play(VOICE_HFSUCCESS,ONCE,1);
			mp.reload=1;
			default_fqp();
			save_param(1);
			save_param(0);
			rt_thread_delay(500);
			NVIC_SystemReset();
		}
		if (ev & INFO_EVENT_PROTECT_ON) {
			rt_time_t cur_time = time(RT_NULL);
			rt_kprintf("liji protect %s\r\n",ctime(&cur_time));
			SetStateIco(1,0);
			SetStateIco(0,1);
			rt_hw_led_on(ARM_LED);
			cur_status = 1;				
			g_flag = 1;		
			//Wtn6_Play(VOICE_BUFANG,ONCE);
			rt_kprintf("bufang ok %d\r\n",g_yanshi);
			handle_led(TYPE_PROTECT_ON);
			pgm_ctl(5);
			if (fqp.is_alarm_voice)
			{
				GPIO_SetBits(GPIOE, GPIO_Pin_11);
				rt_thread_delay(100);
				GPIO_ResetBits(GPIOE, GPIO_Pin_11);
			}

			fqp.status=cur_status;
			set_fq_on(fangqu_wire,WIRE_MAX);
			set_fq_on(fangqu_wireless,WIRELESS_MAX);			
			set_fq_on81(fangqu_io,IO_MAX);			
			rt_event_send(&(g_info_event), INFO_EVENT_SAVE_FANGQU);
			cur_time = time(RT_NULL);
			rt_kprintf("\r\n\r\nnow stm32 is protect on %d, %s\r\n\r\n",g_remote_protect,ctime(&cur_time));
			g_sub_event_code = 0x2002;
			g_fq_len = 1;
			g_fq_event[0] = 0xff;
			rt_kprintf("bufang 1 %d %d\r\n",g_yanshi,g_remote_protect);
			//g_operater[5] = 0x10;	
			if (!g_yanshi) {
				if (g_remote_protect != 1)
				{	
					if (time_protect) {
						rt_uint8_t voice[2] ={ VOICE_YAOKONG,VOICE_BUFANG };

						memset(g_operater,0,6);
						g_operater[0] =  0x10;
						time_protect = 0;
						if (duima_key) {
							duima_key=0;
							g_operate_platform = 0xfc;
							g_operater[5] = 0x01;
						}
						else
							g_operate_platform = 0xfd;
						if (g_operate_platform == 0xfd)
							voice[0] = VOICE_DINGSHI;
						else if (g_operate_platform == 0xfc)
							voice[0] = VOICE_ANJIAN;
						Wtn6_JoinPlay(voice,2,1);
					} else {
						command_type = 0;
						memset(g_operater,0,6);
						g_operater[0] =  0x10;
						g_operater[1] = fangqu_wireless[g_index_sub].index;
						g_operate_platform = 0xff;
						rt_uint8_t voice[2] ={ VOICE_YAOKONG,VOICE_BUFANG };
						Wtn6_JoinPlay(voice,2,1);
						rt_kprintf("play yaokong bufang voice\r\n");
					}
					upload_server(CMD_SUB_EVENT);
					upload_sub_status();
				} else {
					rt_uint8_t voice[2] ={ VOICE_ZHONGXIN,VOICE_BUFANG };
					if (g_operate_platform==0x20)
						voice[0] = VOICE_WEIXIN;//weixin TDO
					else if (g_operate_platform==0x40)
						voice[0] = VOICE_SHOUJI;
					Wtn6_JoinPlay(voice,2,1);			   
					if (time_protect) {
						g_operate_platform = 0xfd;
						memset(g_operater,0,6);
						g_operater[0] =  0x10;
						time_protect = 0;
						upload_server(CMD_SUB_EVENT);
						upload_sub_status();
					}
				}
			}
			else 
			{
				if (g_remote_protect != 1)
				{	
					rt_uint8_t voice[2] ={ VOICE_YAOKONG,VOICE_BUFANG };
					if (!time_protect) {
						Wtn6_JoinPlay(voice,2,1);
					} else {
						voice[0] = VOICE_ANJIAN;
						Wtn6_JoinPlay(voice,2,1);
						rt_kprintf("s button yanshi bufang\r\n");
					}
				} else {
					rt_uint8_t voice[2] ={ VOICE_ZHONGXIN,VOICE_BUFANG };
					Wtn6_JoinPlay(voice,2,1);
				}

			}
			time_protect =0;
			g_yanshi = 0;
		}		
		if (ev & INFO_EVENT_DELAY_PROTECT_ON) {
			SetStateIco(1,0);
			SetStateIco(0,1);
			g_delay_out = fqp.delya_out;
			if (fqp.alarm_voice_time>0)
				g_alarm_voice = fqp.alarm_voice_time*60-ADJUST_TIME;
			else
				g_alarm_voice = 0;
			g_yanshi = 1;
			g_flag = 1;	
			s_bufang=1;		
			g_sub_event_code = 0x2002;
			g_fq_len = 1;
			g_fq_event[0] = 0xff;
			if (g_remote_protect != 1)
			{	

				if (time_protect) {

					rt_uint8_t voice[2] ={ VOICE_YAOKONG,VOICE_BUFANG };
					memset(g_operater,0,6);
					g_operater[0] =  0x10;
					if (duima_key) {
						duima_key=0;
						g_operate_platform = 0xfc;
						g_operater[5] = 0x01;
					}
					else
						g_operate_platform = 0xfd;
					if (g_operate_platform == 0xfd)
						voice[0] = VOICE_DINGSHI;
					else if (g_operate_platform == 0xfc)
						voice[0] = VOICE_ANJIAN;
					Wtn6_JoinPlay(voice,2,1);
					//time_protect = 0;
				} else {
					command_type = 0;
					memset(g_operater,0,6);
					g_operater[0] =  0x10;
					g_operater[1] = fangqu_wireless[g_index_sub].index;
					g_operate_platform = 0xff;
					rt_uint8_t voice[2] ={ VOICE_YAOKONG,VOICE_BUFANG };
					Wtn6_JoinPlay(voice,2,1);
				}
				upload_server(CMD_SUB_EVENT);
				upload_sub_status();
			} else {

				if (time_protect) {
					Wtn6_Play(VOICE_BUFANG,ONCE,1);
					g_operate_platform = 0xfd;
					memset(g_operater,0,6);
					g_operater[0] =  0x10;
					//time_protect = 0;
					upload_server(CMD_SUB_EVENT);
					upload_sub_status();
				} else
				{
					rt_uint8_t voice[2] ={ VOICE_ZHONGXIN,VOICE_BUFANG };					
					if (g_operate_platform==0x20)
						voice[0] = VOICE_WEIXIN;//weixin TDO
					else if (g_operate_platform==0x40)
						voice[0] = VOICE_SHOUJI;
					Wtn6_JoinPlay(voice,2,1);			   
				}
			}
			rt_thread_delay(200);
			if (g_delay_out>10)
				Wtn6_Play(VOICE_YANSHIBF,LOOP,0);
			rt_kprintf("yanshi delay out %d, alarm voice %d\r\n",g_delay_out,g_alarm_voice);
		}
		if (ev & INFO_EVENT_PROTECT_OFF) {
			rt_kprintf("get protect off\r\n");
			alarm_led=0;
			SetStateIco(0,0);
			SetStateIco(2,0);
			SetStateIco(1,1);
			rt_hw_led_off(ARM_LED);	
			rt_hw_led_off(ALARM_LED);	
			//Wtn6_Play(VOICE_CHEFANG,ONCE);
			handle_led(TYPE_PROTECT_OFF);
			pgm_ctl(6);
			rt_hw_led_off(PGM3_LED);
			rt_hw_led_off(PGM4_LED);
			if (fqp.is_alarm_voice)
			{
				GPIO_SetBits(GPIOE, GPIO_Pin_11);
				rt_thread_delay(100);
				GPIO_ResetBits(GPIOE, GPIO_Pin_11);
			}		
			rt_kprintf("\r\n\r\nnow stm32 is protect off %d %d\r\n\r\n",g_remote_protect,time_protect);
			g_sub_event_code = 0x2001;
			g_fq_len = 1;
			g_fq_event[0] = 0xff;
	set_fq_off81(fangqu_io,IO_MAX);

			//g_operater[5] = 0x10;
			if (g_remote_protect!=1)
			{	
				if (time_protect) {
					rt_uint8_t voice[2] ={ VOICE_YAOKONG,VOICE_CHEFANG };
					memset(g_operater,0,6);
					g_operater[0] = 0x10;
					if (duima_key) {
						duima_key=0;
						g_operate_platform = 0xfc;
						g_operater[5] = 0x01;
					}
					else
						g_operate_platform = 0xfd;
					time_protect = 0;
					if (g_operate_platform == 0xfd)
						voice[0] = VOICE_DINGSHI;
					else if (g_operate_platform == 0xfc)
						voice[0] = VOICE_ANJIAN;
					Wtn6_JoinPlay(voice,2,1);
					rt_kprintf("voice chefang\r\n");
				}else{
					command_type = 0;
					memset(g_operater,0,6);
					g_operater[0] = 0x10;
					g_operater[1] = fangqu_wireless[g_index_sub].index;
					g_operate_platform = 0xff;
					rt_uint8_t voice[2] ={ VOICE_YAOKONG,VOICE_CHEFANG };
					Wtn6_JoinPlay(voice,2,1);
					rt_kprintf("VOICE_YAOKONG chefang\r\n");
				}
				upload_server(CMD_SUB_EVENT);	
			} else {
				rt_uint8_t voice[2] ={ VOICE_ZHONGXIN,VOICE_CHEFANG };				
				if (g_operate_platform==0x20)
					voice[0] = VOICE_WEIXIN;//weixin TDO
				else if (g_operate_platform==0x40)
					voice[0] = VOICE_SHOUJI;
				Wtn6_JoinPlay(voice,2,1);
				rt_kprintf("VOICE_SHOUJI chefang\r\n");
#if 0
				if (time_protect) {
					memset(g_operater,0,6);
					g_operater[0] = 0x10;
					g_operate_platform = 0xfd;
					time_protect = 0;
					upload_server(CMD_SUB_EVENT);
				}
#endif
			}
			//entering_ftp_mode	=1;
			g_num=0;
			SetErrorCode(g_num);
			g_alarm_voice=0;
			bell_ctl(0);	
			g_delay_out = 0;
			rt_kprintf("set delay out 0\r\n");
		}

		if (ev & INFO_EVENT_ALARM) {
			rt_kprintf("ALARM command type %d %d\r\n", command_type,g_operationType);
			if (command_type == 4)
				continue;
			g_alarm_reason = 0x1001;
			alarm_led =1;
			//SetStateIco(2,1);

			rt_hw_led_on(ALARM_LED);
			rt_kprintf("cur status %d , is_lamp %d, is_alarm_voice %d, delay_in %d, alarm_voice %d, voiceType %d\r\n",
					cur_status,fqp.is_lamp,fqp.is_alarm_voice,fqp.delay_in,fqp.alarm_voice_time,g_voiceType/*fangqu_wireless[g_index_sub].voiceType*/);
			pgm_ctl(1);
			pgm_ctl(2);
			/*handle alarm voice*/
			if (s1==1) {
				g_alarm_reason = 0x1002;
				if (fqp.is_alarm_voice) {
					if (sub_cmd_type !=2 && sub_cmd_type !=3) {
						bell_ctl(1);
						rt_thread_delay(100);
					}
				}
			} else {

				if (/*fangqu_wireless[g_index_sub].voiceType*/g_voiceType == 0 && fqp.is_alarm_voice && fqp.alarm_voice_time>0)
				{
					if ( /*fangqu_wireless[g_index_sub].*/g_operationType==0 || g_alarmType == 2)
						if (fqp.alarm_voice_time>0)
							g_alarm_voice = fqp.alarm_voice_time*60-ADJUST_TIME;
						else
							g_alarm_voice = 0;

					if (/*fangqu_wireless[g_index_sub].*/g_operationType != 1)
						bell_ctl(1);
				}
			}
			/*handle audio*/
			if (/*fangqu_wireless[g_index_sub].*/g_voiceType == 0) {
				if (s1 == 0 && /*fangqu_wireless[g_index_sub].*/g_operationType !=2) { //non-emergency
					rt_kprintf("non-emergency audio play\r\n");
					if (/*fangqu_wireless[g_index_sub].*/g_operationType == 1 && fqp.delay_in > 0) { //delay mode
						//g_alarm_voice = fqp.alarm_voice_time;
						g_flag=0;
						g_delay_in = fqp.delay_in;
						rt_kprintf("non-emergency audio delay mode %d %d\r\n",fqp.delay_in,g_flag);
						if (g_delay_out ==0)
							Wtn6_Play(VOICE_ALARM2,LOOP,0);
						else
							Wtn6_Play(VOICE_ALARM1,LOOP,0);
					} else {
						rt_kprintf("non-emergency audio normal mode\r\n");
						if (fqp.alarm_voice_time>0)
							g_alarm_voice = fqp.alarm_voice_time*60-ADJUST_TIME;
						else
							g_alarm_voice = 0;
						bell_ctl(1);
						Wtn6_Play(VOICE_ALARM1,LOOP,0);
					}
				} else {
					rt_kprintf("emergency audio play\r\n");
					g_alarm_reason = 0x1003;
					if (s1 == 1) { //s1 switch							
						if (sub_cmd_type == 3)
							g_alarm_reason = 0x0016;
						else
							g_alarm_reason = 0x1002;
						rt_kprintf("s1 audio\r\n");
						//Wtn6_Play(VOICE_FCALARM,ONCE);
					} else {
						rt_kprintf("non-s1 audio \r\n");
						if (fqp.alarm_voice_time>0)
							g_alarm_voice = fqp.alarm_voice_time*60-ADJUST_TIME;
						else
							g_alarm_voice = 0;
						rt_uint8_t action = ONCE;
						if (g_alarm_voice >0)
							action = LOOP;
						else
							action = ONCE;
						if (g_alarmType == 0) {//normal		
							rt_kprintf("normal\r\n");
							Wtn6_Play(VOICE_ALARM1,action,1);
						} else if (g_alarmType == 1) {//fire
							rt_kprintf("fire\r\n");
							Wtn6_Play(VOICE_YANSHIBF,action,1);
						} else if (g_alarmType == 2) {//emergency
							rt_kprintf("emergency\r\n");
							Wtn6_Play(VOICE_JJALARM,action,1);
						} else if (g_alarmType == 3) {//medical
							rt_kprintf("medical\r\n");
							Wtn6_Play(VOICE_YLALARM,action,1);
						} else if (g_alarmType == 4) {
							rt_kprintf("4 \r\n");
							Wtn6_Play(VOICE_JIAOLIUDD,action,1);
						} else if (g_alarmType == 5) {
							rt_kprintf("4 \r\n");
							Wtn6_Play(VOICE_RQALARM,action,1);
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
			g_alarm_fq = /*fangqu_wireless[g_index_sub].*/g_fq_index;
			if (s1 || g_operationType != 1 || fqp.delay_in == 0) {
				upload_server(CMD_ALARM);				
			}
		}

		if (ev & INFO_EVENT_SHOW_NUM) {
			SetErrorCode(g_num);
		}
		if (ev & INFO_EVENT_SAVE_FANGQU) {
			if (wire_code) {
				Wtn6_Play(VOICE_DUIMA,ONCE,1);
				wire_code=0;
			}
			save_param(1);
		}
		if (ev & INFO_EVENT_SAVE_MAIN) {
			save_param(0);
		}
		if (ev & INFO_EVENT_SAVE_HWV) {
			save_param(2);
		}
		if (ev & INFO_EVENT_MUTE) {
			rt_kprintf("do mute\r\n");
			Stop_Playing();
			bell_ctl(0);
		}	
		if (ev & INFO_EVENT_ASYNC_EVENT) {
			g_alarm_reason = g_async_alarm;/*0x0017;*/
			upload_server(g_async_event/*0x0004*/); 	
		}
		//rt_mutex_release(&g_stm32_lock);
	}
}
void handle_login_ack(rt_uint8_t *cmd)
{
	rt_kprintf("ack_type \tlogin ack\r\n");
	rt_kprintf("status \t\t%x\r\n",cmd[0]);
	rt_kprintf("Server Time \t%08x\r\n",cmd[1]<<24|cmd[2]<<16|cmd[3]<<8|cmd[4]<<0);
	adjust_time(cmd+1);
	set_alarm_now();
	rt_kprintf("len \t\t%d\r\n",cmd[5]);
	if (cmd[5] != 0) {
		rt_kprintf("new IP: \t\t");
		for (int i=6;i<cmd[5]+6;i++)
			rt_kprintf("%c",cmd[i]);
		/*reconnect with new ip*/
		update_ip_list(cmd+6,cmd[5]);
		rt_event_send(&(g_info_event), INFO_EVENT_SAVE_MAIN);
	} else {
		g_net_state = NET_STATE_LOGED;
	}
	rt_kprintf("\r\n");
	if (cmd[cmd[5]+6] == 1) {
						if((cur_status || (!cur_status && g_delay_out!=0) || g_alarm_voice))
						{
							cur_status = 0;
							g_alarmType =0;
							g_delay_out = 0;
							g_alarm_voice = 0;
							g_delay_in = 0;
							fqp.status=cur_status;
							s1=0;
							//time_protect = 1;
							//duima_key=1;
							g_mute=0;
							g_remote_protect=0;
							rt_kprintf("switch protect off\r\n");
							handle_protect_off();
							//s_cnt=e_cnt=rt_tick_get();
						}
		
	} else if (cmd[cmd[5]+6] == 2){
						if (!cur_status && g_delay_out==0)
						{
							rt_kprintf("switch protect on\r\n");
							g_remote_protect=0;
							g_sub_event_code = 0x2002;
							//time_protect = 1;
							//duima_key=1;
							//s_bufang=1;
							handle_protect_on();
							//s_cnt=e_cnt=rt_tick_get();
						}
	
	}
	if (cmd[0] != 0) {
		g_num = cmd[0] + 0x10;		
		rt_event_send(&(g_info_event), INFO_EVENT_SHOW_NUM);	
		SetStateIco(3,1);
		rt_kprintf("imei failed, restart m26 ...\r\n");
		in_qiact=0;
		qiact_times=0;
		m26_restart_flag = 0;
		g_heart_cnt=0;
		g_net_state = NET_STATE_UNKNOWN;
		pcie_switch(g_module_type);
	} else {
		g_num = 0;
		rt_event_send(&(g_info_event), INFO_EVENT_SHOW_NUM);	
	}
	default_fqp_t2();
}
static int object_name_maxlen(struct rt_list_node *list)
{
	struct rt_list_node *node;
	struct rt_object *object = NULL;
	int max_length = 0, length;

	rt_enter_critical();
	for (node = list->next; node != list; node = node->next)
	{
		object = rt_list_entry(node, struct rt_object, list);

		length = rt_strlen(object->name);
		if (length > max_length) max_length = length;
	}
	rt_exit_critical();

	if (max_length > RT_NAME_MAX || max_length == 0) max_length = RT_NAME_MAX;

	return max_length;
}

rt_inline void object_split(int len)
{
	while (len--) rt_kprintf("-");
}

long _list_thread(struct rt_list_node *list)
{
	int maxlen;
	rt_uint8_t *ptr;
	struct rt_thread *thread;
	struct rt_list_node *node;

	maxlen = object_name_maxlen(list);

	rt_kprintf("%-*.s pri  status      sp     stack size max used left tick  error\n", maxlen, "thread"); object_split(maxlen);
	rt_kprintf(     " ---  ------- ---------- ----------  ------  ---------- ---\n");
	for (node = list->next; node != list; node = node->next)
	{
		rt_uint8_t stat;
		thread = rt_list_entry(node, struct rt_thread, list);
		rt_kprintf("%-*.*s %3d ", maxlen, RT_NAME_MAX, thread->name, thread->current_priority);

		stat = (thread->stat & RT_THREAD_STAT_MASK);
		if (stat == RT_THREAD_READY)        rt_kprintf(" ready  ");
		else if (stat == RT_THREAD_SUSPEND) rt_kprintf(" suspend");
		else if (stat == RT_THREAD_INIT)    rt_kprintf(" init   ");
		else if (stat == RT_THREAD_CLOSE)   rt_kprintf(" close  ");

		ptr = (rt_uint8_t *)thread->stack_addr;
		while (*ptr == '#')ptr ++;

		rt_kprintf(" 0x%08x 0x%08x    %02d%%   0x%08x %03d\n",
				thread->stack_size + ((rt_uint32_t)thread->stack_addr - (rt_uint32_t)thread->sp),
				thread->stack_size,
				(thread->stack_size - ((rt_uint32_t) ptr - (rt_uint32_t) thread->stack_addr)) * 100
				/ thread->stack_size,
				thread->remaining_tick,
				thread->error);
	}

	return 0;
}
extern struct rt_object_information rt_object_container[];
void prepare_upgrade(rt_uint8_t *cmd, rt_uint8_t type)
{
	g_crc = (cmd[7] << 8)|cmd[8];
	if (g_ftp != RT_NULL)
		rt_free(g_ftp);
	g_ftp = rt_malloc((cmd[9] + 1)*sizeof(rt_uint8_t));
	rt_kprintf("server crc\t %x\r\n",g_crc);
	rt_kprintf("len %d\r\n", cmd[9]);
	rt_memset(g_ftp,0,cmd[9]+1);
	memcpy(g_ftp,cmd+10,cmd[9]);
	rt_kprintf("ftp addrss %s\r\n",g_ftp);

	if (!cur_status) {					
		g_exit_reason = 0x01;
		upload_server(CMD_EXIT);
		rt_thread_delay(200);
		entering_ftp_mode = 1;						
		upgrade_type = type;
		HtbLcdClear();
		Wtn6_Play(VOICE_UPDATE,ONCE,1);
		if (((g_index == 1 && g_type1 == PCIE_2_NBIOT) ||
					(g_index == 0 && g_type0 == PCIE_1_NBIOT))
		   )
		{
			entering_ftp_mode_nbiot=1;
			g_nbiot_update_len = 0;
			nbiot_module = 1;
		}
	}

}
void handle_heart_beat_ack(rt_uint8_t *cmd)
{
	rt_uint32_t ts = cmd[0]<<24|cmd[1]<<16|cmd[2]<<8|cmd[3]<<0;
	rt_kprintf("ack_type \theart ack\r\n");	
	rt_kprintf("cur beat alarm %x %x\r\n",RTC_GetCounter(),RTC_GetAlarm());
	rt_kprintf("Server Time \t%s",ctime(&ts));
	adjust_time(cmd);
	rt_kprintf("type \t\t%d\r\n",cmd[4]);
	if (cmd[4] != 0) {
		rt_uint16_t v = (cmd[5]<<8)|cmd[6];
		switch (cmd[4]) {
			case 0x01:			
				rt_kprintf("new APP version: \t%d",v);
				g_app_v = v;
				prepare_upgrade(cmd,1);
				break;
			case 0x02:			
				rt_kprintf("new SokcetIP version: \t%d",v);
				mp.socketAddressVersion = v;
				break;
			case 0x03:			
				rt_kprintf("new UpdateIP version: \t%d",v);
				mp.updateAddressVersion = v;
				break;
			case 0x04:
				rt_kprintf("new BOOT version: \t%d", v);
				hwv.bootversion0 = (v>>8)&0xff;
				hwv.bootversion1 = v&0xff;
				g_app_v = v;
				prepare_upgrade(cmd,0);
				break;
		}
		if (cmd[4] == 2 || cmd[4] == 3) {
			g_addr_type = cmd[4];
			upload_server(CMD_ASK_ADDR);
		}		
	}
	//if (g_heart_cnt > 1)
	//	g_heart_cnt--;
	if (g_need_reset_rtc) {
		reset_alarm_now();
	}
	g_heart_cnt = 0;
	show_memory_info();

	check_off_line_alarm();
	_list_thread(&rt_object_container[RT_Object_Class_Thread].object_list);

	/*if (!gdo_level()) 
	  {
	  rt_kprintf("poll release sem\r\n");
	  rt_sem_release(&(cc1101_rx_sem));
	  }*/
	rt_kprintf("%s%s %s\r\n", APP_BUILD_TIME,__DATE__,__TIME__);
	if (g_all_fq_num != g_all_fq_num_bak) {
		rt_kprintf("need upgrade fq list to server\r\n");
		g_all_fq_num_bak = g_all_fq_num;
		upload_server(CMD_ASK_SUB_ACK);
	}
}
void handle_t_common_ack(rt_uint8_t *cmd)
{
	rt_kprintf("ack_type \tcommon ack\r\n");
	rt_kprintf("status %d\t\r\n",cmd[0]);
}
void handle_get_address_ack(rt_uint8_t *cmd)
{
	rt_uint16_t ofs = 0;
	rt_kprintf("ack_type \taddr ack\r\n");
	rt_kprintf("addr type \t%d\r\n", cmd[0]);
	rt_kprintf("ip num \t\t%d\r\n", cmd[1]);
	ofs = 3;
	for (int i = 0; i<cmd[1]; i++) {
		for(int j = ofs; j<cmd[ofs-1]+ofs;j++)
			rt_kprintf("%c", cmd[j]);
		rt_kprintf("\r\n");
		ofs += cmd[ofs-1]+1;
	}
}
void handle_info_ack(rt_uint8_t *cmd)
{
	rt_kprintf("get server ack %d\r\n", cmd[0]);
}
void handle_ask_sub(rt_uint8_t *cmd)
{
	rt_kprintf("cmd_type \task sub\r\n");
	rt_kprintf("operate platform \t%x\r\n", cmd[0]);
	rt_kprintf("operater \t%x%x%x%x%x%x\r\n",
			cmd[1],cmd[2],cmd[3],cmd[4],cmd[5],cmd[6]);
	g_operate_platform = cmd[0];
	memcpy(g_operater,cmd+1,6);
	/*build sub infor*/
	upload_server(CMD_ASK_SUB_ACK);
}
void handle_ask_main(rt_uint8_t *cmd)
{
	rt_kprintf("cmd_type \task main\r\n");
	rt_kprintf("operate platform \t%d\r\n", cmd[0]);
	rt_kprintf("operater \t%x%x%x%x%x%x\r\n",
			cmd[1],cmd[2],cmd[3],cmd[4],cmd[5],cmd[6]);
	g_operate_platform = cmd[0];
	memcpy(g_operater,cmd+1,6);
	/*build main infor*/
	upload_server(CMD_ASK_MAIN_ACK);
}
void handle_set_main(rt_uint8_t *cmd)
{
	rt_kprintf("cmd_type \tset main\r\n");
	fqp.alarm_voice_time = (cmd[0]&0xf0)>>4;
	fqp.audio_vol = cmd[0]&0x0f;
	fqp.is_lamp = (cmd[1]&0xf0)>>4;
	fqp.PGM0 = (cmd[2]&0xf0)>>4;
	fqp.PGM1 = (cmd[2]&0x0f);
	fqp.is_check_AC = (cmd[3]&0x80)>>7;
	fqp.is_check_DC = (cmd[3]&0x40)>>6;
	fqp.is_alarm_voice = (cmd[3]&0x20)>>5;
	fqp.is_unprotect_voice = (cmd[3]&0x10)>>5;
	//if (((cmd[4]<<8)|cmd[5]) != 0xffff)
	fqp.auto_bufang = ((cmd[4]<<8)|cmd[5])<<16;
	//if (((cmd[6]<<8)|cmd[7]) != 0xffff)
	fqp.auto_bufang |= (cmd[6]<<8)|cmd[7];
	//if (((cmd[8]<<8)|cmd[9]) != 0xffff)
	fqp.auto_chefang = ((cmd[8]<<8)|cmd[9])<<16;		
	//if (((cmd[10]<<8)|cmd[11]) != 0xffff)
	fqp.auto_chefang |= (cmd[10]<<8)|cmd[11];
	g_operate_platform = cmd[12];
	memcpy(g_operater,cmd+13,6);
	Wtn6_Set_Volumne(fqp.audio_vol|0xe0);
	rt_kprintf("operate platform \t%x\r\n", cmd[12]);
	rt_kprintf("operater \t%x%x%x%x%x%x\r\n",
			cmd[13],cmd[14],cmd[15],cmd[16],cmd[17],cmd[18]);
	set_alarm_now();
	rt_kprintf("set main alarm %x %x\r\n",RTC_GetCounter(),RTC_GetAlarm());
	rt_event_send(&(g_info_event), INFO_EVENT_SAVE_FANGQU);
	rt_thread_delay(100);
	/*build proc main ack*/

	/*execute cmd*/
	upload_server(CMD_ASK_MAIN_ACK);
}
void handle_set_unprot(rt_uint8_t *cmd)
{
	rt_kprintf("cmd_type \tset unprotect\r\n");
	fqp.unprot_time = ((cmd[0]<<8)|cmd[1])<<16;
	g_operate_platform = cmd[2];
	memcpy(g_operater,cmd+3,6);
	rt_kprintf("operate platform \t%x\r\n", cmd[2]);
	rt_kprintf("operater \t%x%x%x%x%x%x\r\n",
			cmd[3],cmd[4],cmd[5],cmd[6],cmd[7],cmd[8]);
	set_alarm_now();
	rt_kprintf("set unprotect alarm %x %x\r\n",RTC_GetCounter(),RTC_GetAlarm());
	rt_event_send(&(g_info_event), INFO_EVENT_SAVE_FANGQU);
	rt_thread_delay(100);
	/*build proc main ack*/

	/*execute cmd*/
	upload_server(CMD_SET_PROT_ACK);
}
void handle_set_sub(rt_uint8_t *cmd)
{
	int i;
	rt_kprintf("cmd_type \tset sub\r\n");
	//if (cmd[0] != 0)
	fqp.delya_out= cmd[0];
	//if (cmd[1] != 0)
	fqp.delay_in = cmd[1];
	if (cmd[2] != 0) {
		for (i=3; i<cmd[2]*14+3;i+=14) {
			if (cmd[i+3] == 0xff) /*delete fq*/
			{
				delete_fq(cmd[i],cmd[i+1]);	
			} else {
				rt_kprintf("edit fq %x %x %x\r\n",cmd[i],cmd[i+1],cmd[i+2]);
				edit_fq(cmd[i],cmd[i+1],cmd[i+2],cmd[i+3],
						(cmd[i+4]<<24)|(cmd[i+5]<<16)|(cmd[i+6]<<8)|cmd[i+7],
						(cmd[i+8]<<8)|cmd[i+9],
						(cmd[i+10]<<24)|(cmd[i+11]<<16)|(cmd[i+12]<<8)|cmd[i+13]
					   );
			}
		}
	}
	i=cmd[2]*14+3;
	g_operate_platform = cmd[i];
	memcpy(g_operater,cmd+i+1,6);

	rt_kprintf("operate platform \t%x\r\n", cmd[i]);
	rt_kprintf("operater \t%x%x%x%x%x%x\r\n",
			cmd[i+1],cmd[i+2],cmd[i+3],cmd[i+4],cmd[i+5],cmd[i+6]);
	rt_event_send(&(g_info_event), INFO_EVENT_SAVE_FANGQU);
	rt_thread_delay(100);
	if (mp.reload & 0x02) {
		mp.reload = 0x00;
		rt_event_send(&(g_info_event), INFO_EVENT_SAVE_MAIN);
	}
	/*build proc main ack*/
	upload_server(CMD_ASK_SUB_ACK);

	/*execute cmd*/
}

void handle_proc_main(rt_uint8_t *cmd)
{
	rt_kprintf("cmd_type \tproc main\r\n");
	rt_kprintf("proc code \t");
	switch (cmd[0]) {
		case 1:
			rt_kprintf("restart stm32\r\n");
			g_main_event_code = 0x200b;
			break;
		case 2:
			rt_kprintf("factory reset\r\n");
			g_main_event_code = 0x2006;
			break;
		case 3:
			rt_kprintf("mute alarm\r\n");
			g_main_event_code = 0x200c;
			break;
		case 4:
			rt_kprintf("pgm ctl %d\r\n",cmd[1]);
			g_main_event_code = 0x200d;
			break;
	}	
	rt_kprintf("operate platform \t%d\r\n", cmd[2]);
	rt_kprintf("operater \t%x%x%x%x%x%x\r\n",
			cmd[3],cmd[4],cmd[5],cmd[6],cmd[7],cmd[8]);

	g_operate_platform = cmd[2];
	memcpy(g_operater,cmd+3,6);
	/*build proc main ack*/
	upload_server(CMD_MAIN_EVENT);
	rt_thread_delay(300);
	/*execute cmd*/
	if (cmd[0] == 2) {
		//dfs_mkfs("elm","sd0");		
		mp.reload=1;
		default_fqp();
		save_param(1);
		save_param(0);
		Wtn6_Play(VOICE_HFSUCCESS,ONCE,1);
	}
	if (cmd[0] == 2 || cmd[0] == 1)
		NVIC_SystemReset();
	if (cmd[0] == 3)
	{
		rt_hw_led_off(AUX_LED0);
		rt_hw_led_off(AUX_LED1);
		bell_ctl(0);
	}

	if (cmd[1] !=0 && cmd[0] == 4) {
		pgm_ctl2(cmd[1]);
	}
}

void handle_proc_sub(rt_uint8_t *cmd)
{
	int ofs;
	rt_uint8_t flag = 0;
	rt_kprintf("cmd_type \tproc sub\r\n");
	rt_kprintf("proc code \t%x\r\n", cmd[0]);
	rt_kprintf("fq len \t\t%d\r\n", cmd[1]);
	if (cmd[1] == 1)
	{
		g_fq_event[0] = cmd[2];
		rt_kprintf("proc fq \t%d\r\n", g_fq_event[0]);
		ofs = 3;
	} else {
		memcpy(g_fq_event, cmd+2,10);
		rt_kprintf("proc fq \t%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\r\n",
				g_fq_event[0],g_fq_event[1],g_fq_event[2],g_fq_event[3],g_fq_event[4],
				g_fq_event[5],g_fq_event[6],g_fq_event[7],g_fq_event[8],g_fq_event[9]);
		ofs = 12;
	}
	rt_kprintf("operate platform \t%d\r\n", cmd[ofs]);
	rt_kprintf("operater \t%x%x%x%x%x%x\r\n",
			cmd[ofs+1],cmd[ofs+2],cmd[ofs+3],cmd[ofs+4],cmd[ofs+5],cmd[ofs+6]);
	g_operate_platform = cmd[ofs];
	memcpy(g_operater,cmd+ofs+1,6);
	g_remote_protect = 1;	
	/*build proc sub ack*/
	g_fq_len=cmd[1];
	if (cmd[0] == 1)
		g_sub_event_code = 0x2001;
	else if (cmd[0] == 2)
		g_sub_event_code = 0x2002;	
	else if (cmd[0] == 3)
		g_sub_event_code = 0x2004;	
	else if (cmd[0] == 4)
		g_sub_event_code = 0x2003;
	else if (cmd[0] == 5)
		g_sub_event_code = 0x200D;
	else if (cmd[0] == 6)
		g_sub_event_code = 0x2005;
	else if (cmd[0] == 7)
		g_sub_event_code = 0x200e;
	/*execute cmd*/
	if (cmd[1] == 1) {
		if (cmd[2] == 0xff) {
			/*proc stm32*/
			if (cmd[0] == 0x01) {
				if((cur_status || (!cur_status && g_delay_out!=0) || g_alarm_voice))
				{					
					upload_server(CMD_SUB_EVENT);
					if (g_main_state != 1) {
						cur_status = 0;
						g_alarmType =0;
						g_delay_out = 0;
						g_alarm_voice = 0;
						g_delay_in = 0;
						fqp.status=cur_status;
						s1=0;
						handle_protect_off();
					}
				} else {
					flag=1;
					//Wtn6_Play(VOICE_ERRORTIP,ONCE);
				}
			} else if (cmd[0] == 0x02) {
				if (!cur_status && g_delay_out==0)
				{
					//g_sub_event_code = 0x2002;
					upload_server(CMD_SUB_EVENT);
					upload_sub_status();
					if (g_main_state != 1)
						handle_protect_on();
				} else {
					flag = 1;
					//Wtn6_Play(VOICE_ERRORTIP,ONCE);
				}
			} else if (cmd[0] == 0x07) {
				for (int i=0; i<WIRE_MAX; i++) 
					if (fangqu_wire[i].slave_batch != 0) {
						set_sub_wire_led(i, 10);
					}
			}
		} else {
			/*proc signle fq*/
			//proc_fq(cmd+2, 1, cmd[0]);
			if (g_main_state != 1 && cmd[0] == 0x02)
				handle_protect_on();
			proc_detail_fq(cmd[2], cmd[0]);
			if (!(fangqu_wireless[cmd[2]-2].slave_model == 0xd001 && cmd[0] == 0x03) && !flag)
				upload_server(CMD_SUB_EVENT);
			if (cmd[0] == 5)
				upload_server(CMD_ASK_SUB_ACK);
			if (cmd[0] == 7)
				set_sub_wire_led(cmd[2]-50, 10);
		}
	} else {
		/*proc multi fq*/
		if (g_main_state != 1 && cmd[0] == 0x02)
			handle_protect_on();
		proc_fq(cmd+2, 10, cmd[0]);
		if (g_operate_platform == 0x20 && (cmd[0] == 1 || cmd[0] == 2)) {
			rt_uint8_t voice[2] ={ VOICE_FENQU,VOICE_BUFANG };
			if (cmd[0] == 1)
				voice[1] = VOICE_CHEFANG;
			Wtn6_JoinPlay(voice,2,1);
		}
		if (!(fangqu_wireless[cmd[2]-2].slave_model == 0xd001 && cmd[0] == 0x03) && !flag)
			upload_server(CMD_SUB_EVENT);
		if (cmd[0] == 5)
			upload_server(CMD_ASK_SUB_ACK);
		if (cmd[0] == 7)
			proc_wire_fq(cmd+2, 10, cmd[0]);
	}
	rt_event_send(&(g_info_event), INFO_EVENT_SAVE_FANGQU);
	rt_thread_delay(100);
	//if (!(fangqu_wireless[cmd[2]-2].slave_model == 0xd001 && cmd[0] == 0x03) && !flag)
	//upload_server(CMD_SUB_EVENT);
}
void handle_bc26_update(rt_uint8_t *data)
{
	rt_uint32_t all_len = (data[0]<<24)|(data[1]<<16)|(data[2]<<8)|(data[3]<<0);
	rt_uint32_t cur_len = (data[4]<<8)|(data[5]<<0);
	rt_kprintf("cmd_type \tbc26 update\r\n");
	rt_kprintf("all len \t%d\r\n", all_len);
	rt_kprintf("cur len \t%d\r\n", cur_len);
	g_nbiot_update_len += cur_len;
	if (g_nbiot_update_len < all_len)
	{
		if (bc28_down_fd == -1) {
			if (upgrade_type)
				bc28_down_fd = open("/stm32.bin",  O_WRONLY | O_CREAT | O_TRUNC, 0);	
			else
				bc28_down_fd = open("/BootLoader.bin",  O_WRONLY | O_CREAT | O_TRUNC, 0);	
		}
		if (cur_len != write(bc28_down_fd, data+6, cur_len))
		{
			rt_kprintf("write bc28 data failed\n");
			//close(bc28_down_fd);
			//break;
		}
		fsync(bc28_down_fd);
		if (upgrade_type)
			upload_server(CMD_UPDATE);
		else
			upload_server(CMD_UPDATE_BOOT);
	}
	else {
		if (cur_len != write(bc28_down_fd, data+6, cur_len))
		{
			rt_kprintf("write bc28 data failed2\n");
			//close(bc28_down_fd);
			//break;
		}
		fsync(bc28_down_fd);
		close(bc28_down_fd);
		bc28_down_fd=-1;
		if (upgrade_type)
			mp.firmCRC = CRC_check_file("/stm32.bin");
		else
			mp.firmCRC = CRC_check_file("/BootLoader.bin");
		if (mp.firmCRC != g_crc)
		{
			rt_kprintf("App/Boot download failed %x %x\r\n",mp.firmCRC,g_crc);
		} else {
			if (upgrade_type) {
				rt_kprintf("App donwload ok\r\n");
				if (g_app_v!=0)
					mp.firmVersion = g_app_v;
				mp.firmLength = all_len;
				rt_event_send(&(g_info_event), INFO_EVENT_SAVE_MAIN);
				entering_ftp_mode = 0;
				g_heart_cnt=0;
				g_net_state = NET_STATE_UNKNOWN;
				pcie_switch(g_module_type);
			} else {
				rt_kprintf("Boot donwload ok\r\n");
				rt_event_send(&(g_info_event), INFO_EVENT_SAVE_HWV);
				/* real update boot*/
				rt_kprintf("going to upgrade Boot\r\n");
				write_flash(0x08000000, "/BootLoader.bin", g_nbiot_update_len);
			}
			if (!cur_status) {
				rt_thread_delay(500);
				NVIC_SystemReset();
			}
		}
	}
}
rt_uint8_t handle_packet(rt_uint8_t *data)
{
	int i=0;
	rt_uint8_t water_no = data[0];
	rt_uint16_t packet_type = (data[1]<<8)|data[2];
	rt_uint16_t protocl_v = (data[3]<<8)|data[4];
	rt_uint8_t stm32_id[6];	
	begin_yunduo();
	//net_flow();
	//net_flow_flag=1;
	memcpy(stm32_id, data+5,6);
	rt_time_t cur_time = time(RT_NULL);
	rt_kprintf("\r\nrecv server ok ========> %s\r\n",ctime(&cur_time));
	rt_kprintf("=================================================>\r\nwater no \t%d\r\n", water_no);
	rt_kprintf("pacet type \t%x\r\n", packet_type);
	rt_kprintf("protol version \t%d\r\n", protocl_v);
	rt_kprintf("sn \t\t%02x%02x%02x%02x%02x%02x\r\n",
			stm32_id[0],stm32_id[1],stm32_id[2],
			stm32_id[3],stm32_id[4],stm32_id[5]);
	if (memcmp(stm32_id,mp.roProperty.sn,6) != 0)
	{
		rt_kprintf("packet not for us \t%02x%02x%02x%02x%02x%02x\r\n",
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
		case ALARM_TRAP_ACK:
		case FQ_OP_ACK:
		case MAIN_OP_ACK:
			handle_t_common_ack(data+11);
			break;
		case GET_ADDRESS_ACK:
			handle_get_address_ack(data+11);
			break;

		case CMD_PROC_SUB:
			handle_proc_sub(data+11);
			break;
		case CMD_PROC_MAIN:
			handle_proc_main(data+11);
			break;
		case CMD_SET_SUB:
			handle_set_sub(data+11);
			break;
		case CMD_SET_MAIN:
			handle_set_main(data+11);
			break;		
		case CMD_ASK_SUB:
			handle_ask_sub(data+11);
			break;
		case CMD_ASK_MAIN:
			handle_ask_main(data+11);
			break;
		case UPDATE_ACK:
		case UPDATE_BOOT_ACK:
			handle_bc26_update(data+11);
			break;
		case INFO_ACK:
			handle_info_ack(data+11);
			break;
		case CMD_SET_UNPROT:
			handle_set_unprot(data+11);
			break;
		default:
			rt_kprintf("unknown packet type\r\n");
			break;
	}
	rt_kprintf("<=================================================\r\n");
	return 1;
}
