#include "ftpc.h"
#include <board.h>
#include <rtthread.h>
#include <rtdevice.h>
#include <stdio.h>
#include <unistd.h> 
#include <board.h>
#include <rtthread.h>
#include <rtdevice.h>
#include <dfs_posix.h>
#include "led.h"
#include <string.h>
#include "m26.h"
#include "pcie.h"
#include "bsp_misc.h"
#include "master.h"
#include "prop.h"
#include "lcd.h"
#include <ctype.h>

extern struct rt_event g_info_event;
extern int stm32_len;
extern rt_uint16_t g_app_v;
extern int down_fd;
extern rt_uint8_t upgrade_type;
extern rt_mp_t server_mp;
extern rt_uint8_t 	cur_status;
extern rt_uint16_t g_crc;
extern rt_uint8_t *g_ftp;
un_l2cval remote_ip;
uint16_t  remote_port;
un_l2cval local_ip;
uint16_t  local_port;
uint8_t connect_state_control_ftpc = 0;
uint8_t connect_state_data_ftpc = 0;
uint8_t gModeActivePassiveflag = 0;
uint8_t gMenuStart = 0;
uint8_t gDataSockReady = 0;
uint8_t gDataPutGetStart = 0;
static uint8_t gMsgBuf[20]={0,};
uint8_t gSend_quit = 0;
struct ftpc ftpc;
struct Command Command;
unsigned int crc_ori = 0;
unsigned int ftp_CRC_check(unsigned char *Data,unsigned short Data_length)
{
	unsigned int mid=0;
	unsigned char times=0;
	unsigned short Data_index=0;
	unsigned int CRC1=0xFFFF;
	if (crc_ori != 0)
		CRC1 = crc_ori;
	while(Data_length)
	{
		CRC1=((unsigned int)(Data[Data_index]))^CRC1;
		for(times=0;times<8;times++)
		{
			mid=CRC1;
			CRC1=CRC1>>1;
			if(mid & 0x0001)
			{
				CRC1=CRC1^0xA001;
			}
		}
		Data_index++;
		Data_length--;
	}
	return CRC1;
}

uint8_t ftpc_run(uint8_t * dbuf, uint8_t *dest_ip, uint16_t dest_port, uint8_t *path)
{
	uint16_t size = 0;
	long ret = 0;
	uint32_t send_byte, recv_byte;
	uint32_t blocklen;
	uint32_t remain_filesize;
	uint32_t remain_datasize;
	uint8_t msg_c;
	uint8_t dat[50]={0,};
	uint32_t totalSize = 0, availableSize = 0;
	wiz_NetInfo gWIZNETINFO;
	ctlnetwork(CN_GET_NETINFO, (void*) &gWIZNETINFO);

	ftpc.dsock_mode = ACTIVE_MODE;

	local_ip.cVal[0] = gWIZNETINFO.ip[0];
	local_ip.cVal[1] = gWIZNETINFO.ip[1];
	local_ip.cVal[2] = gWIZNETINFO.ip[2];
	local_ip.cVal[3] = gWIZNETINFO.ip[3];
	local_port = 35000;
	strcpy(ftpc.workingdir, "/");
	socket(CTRL_SOCK, Sn_MR_TCP, dest_port, 0x0);
	while (1) {
		switch(getSn_SR(CTRL_SOCK))
		{
			case SOCK_ESTABLISHED :
				if(!connect_state_control_ftpc){
					rt_kprintf("%d:FTP Connected\r\n", CTRL_SOCK);
					strcpy(ftpc.workingdir, "/");
					connect_state_control_ftpc = 1;
				}

				if(gMenuStart){
					gMenuStart = 0;
					ftpc.dsock_mode=PASSIVE_MODE;
					sprintf(dat,"PASV\r\n");
					send(CTRL_SOCK, (uint8_t *)dat, strlen(dat));
					Command.First = f_get;
				}
				if(gDataSockReady){
					gDataSockReady = 0;
					switch(Command.First){
						case f_get:
							rt_kprintf(">get file name?");
							//strcpy(dat,"RETR /A1/21/stm32.bin\r\n");
							sprintf(dat,"RETR %s\r\n", path);
							send(CTRL_SOCK, (uint8_t *)dat, strlen(dat));
							break;
						default:
							rt_kprintf("Command.First = default\r\n");
							break;
					}
				}
				if((size = getSn_RX_RSR(CTRL_SOCK)) > 0){ // Don't need to check SOCKERR_BUSY because it doesn't not occur.
					memset(dbuf, 0, _MAX_SS);
					if(size > _MAX_SS) size = _MAX_SS - 1;
					ret = recv(CTRL_SOCK,dbuf,size);
					dbuf[ret] = '\0';
					if(ret != size)
					{
						if(ret==SOCK_BUSY) return 0;
						if(ret < 0){
							rt_kprintf("%d:recv() error:%ld\r\n",CTRL_SOCK,ret);
							ip_close(CTRL_SOCK);
							return ret;
						}
					}
					rt_kprintf("Rcvd Command: %s\r\n", dbuf);
					proc_ftpc((char *)dbuf);
				}
				break;
			case SOCK_CLOSE_WAIT :
				rt_kprintf("%d:CloseWait\r\n",CTRL_SOCK);
				if((ret=disconnect(CTRL_SOCK)) != SOCK_OK) return ret;
				rt_kprintf("%d:Closed\r\n",CTRL_SOCK);
				break;
			case SOCK_CLOSED :
				crc_ori = 0;
				ip_close(CTRL_SOCK);
				return 101;
			case SOCK_INIT :
				rt_kprintf("%d:Opened\r\n",CTRL_SOCK);
				if((ret = connect(CTRL_SOCK, dest_ip, dest_port)) != SOCK_OK){
					rt_kprintf("%d:Connect error\r\n",CTRL_SOCK);
					return ret;
				}
				connect_state_control_ftpc = 0;
				rt_kprintf("%d:Connectting...\r\n",CTRL_SOCK);
				break;
			default :
				break;
		}
		switch(getSn_SR(DATA_SOCK)){
			case SOCK_ESTABLISHED :
				if(!connect_state_data_ftpc){
					rt_kprintf("%d:FTP Data socket Connected\r\n", DATA_SOCK);
					connect_state_data_ftpc = 1;
				}
				if(gDataPutGetStart){
					switch(Command.Second){
						case s_get:
							rt_kprintf("get waiting...\r\n");
							if(strlen(ftpc.workingdir) == 1)
								sprintf(ftpc.filename, "/%s", (uint8_t *)gMsgBuf);
							else
								sprintf(ftpc.filename, "%s/%s", ftpc.workingdir, (uint8_t *)gMsgBuf);
//#if ndefined(F_FILESYSTEM)
						if (upgrade_type)
							down_fd = open("/stm32.bin",  O_WRONLY | O_CREAT | O_TRUNC, 0);			
						else
							down_fd = open("/BootLoader.bin",  O_WRONLY | O_CREAT | O_TRUNC, 0);			
							//ftpc.fr = f_open(&(ftpc.fil), (const char *)ftpc.filename, FA_CREATE_ALWAYS | FA_WRITE);
							//if(ftpc.fr == FR_OK){
							if (down_fd > 0) {
								rt_kprintf("f_open return FR_OK\r\n");
								uint16_t cur_len;
								uint32_t data_len = 0;
								while(1){
									if((remain_datasize = getSn_RX_RSR(DATA_SOCK)) > 0){
										while(1){
											memset(dbuf, 0, _MAX_SS);
											if(remain_datasize > _MAX_SS)	recv_byte = _MAX_SS;
											else	recv_byte = remain_datasize;
											ret = recv(DATA_SOCK, dbuf, recv_byte);
											cur_len = write(down_fd, dbuf, ret);
											//ftpc.fr = f_write(&(ftpc.fil), (const void *)dbuf, (UINT)ret, (UINT *)&blocklen);
											remain_datasize -= cur_len;//blocklen;
										data_len += ret;
											//if(ftpc.fr != FR_OK){
											if (cur_len != ret){
												rt_kprintf("f_write failed\r\n");
												break;
											}/*else
												rt_kprintf("write succ %d %d\n", cur_len,remain_datasize);*/

											if(remain_datasize <= 0)	{
												rt_kprintf("remain_datasize %d\n", remain_datasize);
												break;
											}
										}
										//if(ftpc.fr != FR_OK){
										if (cur_len != ret) {
											rt_kprintf("f_write failed\r\n");
											break;
										}
										rt_kprintf("#");
									}
									else{
										if(getSn_SR(DATA_SOCK) != SOCK_ESTABLISHED)	{
											gSend_quit = 1;
											rt_kprintf("link lost DATA_SOCK\n");
											break;
										}
									}
								}
								rt_kprintf("\r\nFile write finished %d\r\n",data_len);
								//ftpc.fr = f_ip_close(&(ftpc.fil));
								fsync(down_fd);
								close(down_fd);
					if (upgrade_type)
					mp.firmCRC = CRC_check_file("/stm32.bin");
					else
					mp.firmCRC = CRC_check_file("/BootLoader.bin");
					crc_ori = mp.firmCRC;
					if (mp.firmCRC != g_crc)
					{
						rt_kprintf("App or Boot download failed %x %x\r\n",mp.firmCRC,g_crc);
					}
					else {
						rt_kprintf("App donwload ok\r\n");
						if (upgrade_type) {
						if (g_app_v!=0)
							mp.firmVersion = g_app_v;
						mp.firmLength = stm32_len;
						rt_event_send(&(g_info_event), INFO_EVENT_SAVE_MAIN);
						} else {
							hwv.bootversion0 = (g_app_v>>8)&0xff;
							hwv.bootversion1 = g_app_v&0xff;
							rt_event_send(&(g_info_event), INFO_EVENT_SAVE_HWV);
							/* real update boot*/
							rt_kprintf("going to upgrade Boot\r\n");
							write_flash(0x08000000, "/BootLoader.bin", stm32_len);
						}
						if (!cur_status) {
						rt_thread_delay(500);
						rt_kprintf("programing boot done, reseting ...\r\n");
						NVIC_SystemReset();
						}
					}
								gDataPutGetStart = 0;
							}else{
								rt_kprintf("File Open Error: %d\r\n", down_fd);
							}
/*#else
							uint32_t data_len = 0;
							while(1){
								if((remain_datasize = getSn_RX_RSR(DATA_SOCK)) > 0){
									while(1){
										memset(dbuf, 0, _MAX_SS);
										if(remain_datasize > _MAX_SS)
											recv_byte = _MAX_SS;
										else
											recv_byte = remain_datasize;
										ret = recv(DATA_SOCK, dbuf, recv_byte);
										//rt_kprintf("########## dbuf:%s\r\n", dbuf);
										crc_ori = ftp_CRC_check(dbuf, ret);
										remain_datasize -= ret;
										data_len += ret;
										if(remain_datasize <= 0)
											break;
									}
								}else{
									if(getSn_SR(DATA_SOCK) != SOCK_ESTABLISHED) {
										rt_kprintf("total file len %d %04x\r\n", data_len, crc_ori);
										gSend_quit = 1;
										break;
									}
								}
							}
							gDataPutGetStart = 0;
							Command.Second = s_nocmd;
#endif*/
							break;
						default:
							rt_kprintf("Command.Second = default\r\n");
							break;
					}
				}
				break;
			case SOCK_CLOSE_WAIT :
				rt_kprintf("%d:CloseWait\r\n",DATA_SOCK);
				if((ret=disconnect(DATA_SOCK)) != SOCK_OK) return ret;
				rt_kprintf("%d:Closed\r\n",DATA_SOCK);
				break;
			case SOCK_CLOSED :
				if (crc_ori == 0) {
					if(ftpc.dsock_state == DATASOCK_READY){
						if(ftpc.dsock_mode == PASSIVE_MODE){
							rt_kprintf("%d:FTPDataStart, port : %d\r\n",DATA_SOCK, local_port);
							if((ret=socket(DATA_SOCK, Sn_MR_TCP, local_port, 0x0)) != DATA_SOCK){
								rt_kprintf("%d:socket() error:%ld\r\n", DATA_SOCK, ret);
								ip_close(DATA_SOCK);
								return ret;
							}
							local_port++;
							if(local_port > 50000)
								local_port = 35000;
						}else{
							rt_kprintf("%d:FTPDataStart, port : %d\r\n",DATA_SOCK, local_port);
							if((ret=socket(DATA_SOCK, Sn_MR_TCP, local_port, 0x0)) != DATA_SOCK){
								rt_kprintf("%d:socket() error:%ld\r\n", DATA_SOCK, ret);
								ip_close(DATA_SOCK);
								return ret;
							}
							local_port++;
							if(local_port > 50000)
								local_port = 35000;
						}
						ftpc.dsock_state = DATASOCK_START;
					} 
				} else {
					if (gSend_quit) {
						gSend_quit = 0;
						ip_close(DATA_SOCK);
						sprintf(dat,"QUIT\r\n");
						rt_kprintf("send quit to ftp server\r\n");
						send(CTRL_SOCK, (uint8_t *)dat, strlen(dat));
					}
				}
				break;

			case SOCK_INIT :
				rt_kprintf("%d:Opened\r\n",DATA_SOCK);
				if(ftpc.dsock_mode == ACTIVE_MODE){
					if( (ret = listen(DATA_SOCK)) != SOCK_OK){
						rt_kprintf("%d:Listen error\r\n",DATA_SOCK);
						return ret;
					}
					gDataSockReady = 1;
					rt_kprintf("%d:Listen ok\r\n",DATA_SOCK);
				}else{
					if((ret = connect(DATA_SOCK, remote_ip.cVal, remote_port)) != SOCK_OK){
						rt_kprintf("%d:Connect error\r\n", DATA_SOCK);
						return ret;
					}
					gDataSockReady = 1;
				}
				connect_state_data_ftpc = 0;
				break;
			default :
				break;
		}
	}
	return 0;
}

char proc_ftpc(char * buf)
{
	uint16_t Responses;
	uint8_t dat[30]={0,};

	Responses =(buf[0]-'0')*100+(buf[1]-'0')*10+(buf[2]-'0');

	switch(Responses){
		case R_220:	/* Service ready for new user. */
			rt_kprintf("\r\nInput your User ID > ");
			strcpy(dat,"USER minfei\r\n");
			rt_kprintf("\r\n");
			send(CTRL_SOCK, (uint8_t *)dat, strlen(dat));
			break;

		case R_331:	/* User name okay, need password. */
			rt_kprintf("\r\nInput your Password > ");
			strcpy(dat,"PASS minfei123\r\n");
			rt_kprintf("\r\n");
			send(CTRL_SOCK, (uint8_t *)dat, strlen(dat));
			break;
		case R_230:	/* User logged in, proceed */
			rt_kprintf("\r\nUser logged in, proceed\r\n");
			sprintf(dat,"TYPE %c\r\n", TransferBinary);
			ftpc.type = IMAGE_TYPE;
			send(CTRL_SOCK, (uint8_t *)dat, strlen(dat));
			break;
		case R_200:
			if((ftpc.dsock_mode==ACTIVE_MODE)&&gModeActivePassiveflag){
				ftpc.dsock_state = DATASOCK_READY;
				gModeActivePassiveflag = 0;
			}
			else{
				gMenuStart = 1;
			}
			break;
		case R_150:
			switch(Command.First){
				case f_get:
					Command.First = f_nocmd;
					Command.Second = s_get;
					gDataPutGetStart = 1;
					break;
				default :
					rt_kprintf("Command.First = default\r\n");
					break;
			}
			break;
		case R_226:
			gMenuStart = 1;
			break;
		case R_227:
			if (pportc(buf) == -1){
				rt_kprintf("Bad port syntax\r\n");
			}
			else{
				rt_kprintf("Go Open Data Sock...\r\n ");
				ftpc.dsock_mode = PASSIVE_MODE;
				ftpc.dsock_state = DATASOCK_READY;
			}
			break;
		case R_425:
			
						ip_close(DATA_SOCK);
						sprintf(dat,"QUIT\r\n");
						rt_kprintf("send fjfquit to ftp server\r\n");
						send(CTRL_SOCK, (uint8_t *)dat, strlen(dat));
			break;
		default:
			rt_kprintf("\r\nDefault Status = %d\r\n",(uint16_t)Responses);
			gDataSockReady = 1;
			break;
	}
	return 1;
}
int pportc(char * arg)
{
	int i;
	char* tok=0;
	strtok(arg,"(");
	for (i = 0; i < 4; i++)
	{
		if(i==0) tok = strtok(NULL,",\r\n");
		else	 tok = strtok(NULL,",");
		remote_ip.cVal[i] = (uint8_t)atoi(tok);//, 10);
		if (!tok){
			rt_kprintf("bad pport : %s\r\n", arg);
			return -1;
		}
	}
	remote_port = 0;
	for (i = 0; i < 2; i++){
		tok = strtok(NULL,",\r\n");
		remote_port <<= 8;
		remote_port += atoi(tok);//, 10);
		if (!tok){
			rt_kprintf("bad pport : %s\r\n", arg);
			return -1;
		}
	}
	rt_kprintf("ip : %d.%d.%d.%d, port : %d\r\n", remote_ip.cVal[0], remote_ip.cVal[1], remote_ip.cVal[2], remote_ip.cVal[3], remote_port);
	return 0;
}
