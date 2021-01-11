#include <rtthread.h>
#include <stdio.h>
#include "mcu_hid.h"
#include "mcu_cmd.h"
#include "rc4.h"
#include "crc.h"
#include "utils.h"
#include "mem_list.h"
static uint16_t sw_major_version = 0x0000;
static uint16_t sw_minor_version = 0x0001;
static uint16_t sw_patch_version = 0x0000;
static uint32_t sw_build_date = 2020082115;
static uint16_t hw_major_version = 0x0000;
static uint16_t hw_minor_version = 0x0001;
static uint16_t hw_patch_version = 0x0000;
static uint32_t hw_build_date = 2020082514;
static uint8_t heart_ts[2][8];
#define nprintf(fmt, arg...)                                            \
	do {                                                            \
		if (debug)                                              \
		rt_kprintf("[MCU][%s]: " fmt, __func__, ## arg);\
	} while (0)

#define PACKET_LEN 512
#define MCU_QUEUE_SIZE           (256)
#define MCU_MSG_SIZE             (8)

static rt_bool_t use_g_msg = RT_FALSE;
static uint8_t g_msg[512] = {0};
uint8_t host_alive = 0;
mcu_func g_mcu_func[NR_MAX] = {
	NULL,
	&handle_brightness,	/* 0 - 7 */
	&handle_dp_mode,	/* 1:2d_1080, 2:3d_540, 3:3d_1080 */
	&handle_dp_version,	/* 7211 version */
	&handle_host_id,
	&handle_glasses_id,
	&handle_psensor_near,	/* psensor near value */
	&handle_psensor_far,	/* psensor far value */
	&handle_get_temp,	/* temperature */
	&handle_oled_duty,	/* oled duty */
	&handle_sleep_mode,	/* 0:close, 1:open */
	&handle_magnetic,	
	&handle_vsync,
	&handle_env_light,
	&handle_rgb_led,
	&handle_sleep_time,	/* time before glasses enter into sleep mode */
	&handle_dp_new_version,	/* dp ota, new version */
	&handle_dp_ota,		/* dp ota */
	&handle_reset,		/* reset cx3 */
	&handle_brightness_v2,	/* 0 - 120 */
	&handle_temp,		/* 0:close, 1:open */
	&handle_try_oled_power,	/* 0:close, 1:open, used in heartbreat */
	&handle_get_temp_v2,	/* temperature ext */
	&handle_brightness_max,	/* set brightness max */
	&handle_speaker_level,	/* speaker gain value */
	&handle_cpu,
	&handle_rom,
	&handle_ram,
	&handle_left_oled_h_orbit,
	&handle_left_oled_v_orbit,
	&handle_right_oled_h_orbit,
	&handle_right_oled_v_orbit,
	&handle_orbit_adjust,	/* 0:close, 1:open */
	&handle_color,		/* 1:red, 2:blue, 3:white */
	&handle_factory,	/* use default setting */
	&handle_bri_test,
	&handle_machine_id,
	&handle_rgb_reset
};

static void parse_host_cmd(uint8_t *cmd, uint16_t len)
{
	uint32_t crc;
	uint8_t msg[32] = {0};
	uint16_t msg_id, payload_len;

	/* protocl 
	 * |plain           |Security                 |  |plain   |
	 * 0    1..8 9 10   11 12 13 14  15 16  17...N   N+1 .. N+4
	 * STX  TS   MSGID  RESERVED     LEN    PAYLOAD  CRC
	 */

	if (cmd[0] != HOST_CMD_STX) {
		nprintf("invalid cmd %x\r\n", cmd[0]);
		msg[0] = MCU_ERR_UNSUPPORT;
		msg[1] = HOST_CMD;
		goto FAIL;
	}

	if (len < 4) {
		nprintf("invalid len %d\r\n", len);
		msg[0] = MCU_ERR_INV_ARG;
		msg[1] = HOST_CMD;
		goto FAIL;
	}

	crc =   cmd[len-4] |
		(cmd[len-3] << 8) |
		(cmd[len-2] << 16) |
		(cmd[len-1] << 24);
	if (crc != crc32((uint8_t *)cmd, len-4)) {
		nprintf("invalid crc h %x != c %x\r\n", crc,
				crc32((uint8_t *)cmd, len-4));
		msg[0] = MCU_ERR_CRC;
		msg[1] = HOST_CMD;
		goto FAIL;
	}

	/* msg structure 
	   0   1   2 3     4 5          6 7 8 9  10..31
	   err h/d msg_id  payload_len  reserve  payload
	   */

	msg_id = (cmd[CMD_MSGID_1] << 8) | cmd[CMD_MSGID_0];
	if (msg_id == HEART_CMD)
		read_ts_64(heart_ts[1]);

	set_rc4_key(cmd[7]%10, msg_id, cmd+CMD_TS_OFS);
	rc4(cmd+CMD_RESERVE_OFS,
			cmd+CMD_RESERVE_OFS, len - 4 - 1 - 8 - 2);

	if (msg_id == HEART_CMD) {
		rt_memcpy(heart_ts[0], cmd+18, 8);
	}

	msg[0] = MCU_ERR_SUCCESS;
	g_msg[0] = MCU_ERR_SUCCESS;
	msg[1] = HOST_CMD;
	msg[2] = cmd[CMD_MSGID_1];
	msg[3] = cmd[CMD_MSGID_0];
	msg[4] = cmd[CMD_PAYLOAD_LEN_1];
	msg[5] = cmd[CMD_PAYLOAD_LEN_0];
	rt_memcpy(msg+6, cmd+CMD_RESERVE_OFS, 4);
	payload_len = (msg[4] << 8) | msg[5];
	

	if (payload_len <= 22) {
		rt_memcpy(msg+10, cmd+CMD_PAYLOAD_OFS, payload_len);
		use_g_msg = CyFalse;
	} else if (payload_len < 512) {
		nprintf("host payload len %d > 24, save global\r\n",
				payload_len);
		rt_memcpy(g_msg+1, msg+2, 8);
		rt_memcpy(g_msg+9, cmd+CMD_PAYLOAD_OFS, payload_len);
		use_g_msg = CyTrue;
	} else {
		msg[0] = MCU_ERR_NREAL;
		msg[1] = HOST_CMD;
	}

FAIL:	
	if (RT_TRUE != mcu_msg_send(msg))
		nprintf("not enough msg to send\r\n");
}
void build_event(uint8_t *msg, uint16_t msg_id, uint16_t len)
{
	/* msg structure 
	   0   1   2 3     4 5          6 7 8 9  10..31
	   err h/d msg_id  payload_len  reserve  payload
	   */
	msg[0] = 0x00;
	msg[1] = DEVICE_EVENT;
	msg[2] = (msg_id >> 8) & 0xff;
	msg[3] = (msg_id >> 0) & 0xff;
	msg[4] = (len >> 8) & 0xff;
	msg[5] = (len >> 0)& 0xff;
	msg[6] = 0x00;
	msg[7] = 0x00;
	msg[8] = 0x00;
	msg[9] = 0x00;
}

static uint16_t get_rsp_msg_id(uint16_t msg_id)
{
	switch (msg_id)
	{
		case HOST_CMD_GET:
			return HOST_CMD_GET_RSP;
		case HOST_CMD_SET:
			return HOST_CMD_SET_RSP;
		default:
			return msg_id;
	}
	return 0;
}
static uint16_t handle_cmd_id(uint16_t msg_id, uint8_t *cmd, uint16_t cmd_len,
		uint8_t *rsp)
{
	/* err, cmd_id_h, cmd_id_l, xxxx */
	uint16_t cmd_id = cmd[2] << 8 | cmd[1];
	uint16_t payload_len = 3;
	rsp[0] = MCU_ERR_SUCCESS;
	rsp[1] = cmd[1];
	rsp[2] = cmd[2];

	if (cmd_id >= NR_MAX) {
		rsp[0] = MCU_ERR_UNSUPPORT;
		return payload_len;
	}

	if (msg_id == HOST_CMD_GET) {
		if (g_mcu_func[cmd_id] != NULL) {
			payload_len = g_mcu_func[cmd_id](NULL, 0, rsp);
		} else {
			rsp[0] = MCU_ERR_UNSUPPORT;
		}
	} else {
		if (g_mcu_func[cmd_id] != NULL)
			payload_len = g_mcu_func[cmd_id](cmd+3, cmd_len - 3, rsp);
		else
			rsp[0] = MCU_ERR_UNSUPPORT;		
	}

	return payload_len;
}
static uint16_t handle_event(uint16_t msg_id, uint8_t *event, uint16_t event_len,
		uint8_t *rsp)
{
	rt_memcpy(rsp, event, event_len);
	return event_len;
}
static void get_sw_ver()
{
	char *p = (char *)CX3_FW_VERSION;
	char *d = __DATE__;
	char *t = __TIME__;
	char major[2] = {0}, minor[3] = {0}, patch[4] = {0};
	char year[5] = {0};
	uint8_t mon = 0;
	char day[3] = {0};
	major[0] = p[3];
	major[1] = 0;
	minor[0] = p[5];
	minor[1] = p[6];
	minor[2] = 0;
	patch[0] = p[8];
	patch[1] = p[9];
	patch[2] = p[10];
	patch[3] = 0;
	sw_major_version = atoi(major);
	sw_minor_version = atoi(minor);
	sw_patch_version = atoi(patch);
	rt_memcpy(year, d+7, 4);
	rt_memcpy(day, d+4, 2);
	if (rt_strstr(d, "Jan"))
		mon = 1;
	else if (rt_strstr(d, "Feb"))
		mon = 2;
	else if (rt_strstr(d, "Mar"))
		mon = 3;
	else if (rt_strstr(d, "Apr"))
		mon = 4;
	else if (rt_strstr(d, "May"))
		mon = 5;
	else if (rt_strstr(d, "Jun"))
		mon = 6;
	else if (rt_strstr(d, "Jul"))
		mon = 7;
	else if (rt_strstr(d, "Aug"))
		mon = 8;
	else if (rt_strstr(d, "Sep"))
		mon = 9;
	else if (rt_strstr(d, "Oct"))
		mon = 10;
	else if (rt_strstr(d, "Nov"))
		mon = 11;
	else if (rt_strstr(d, "Dec"))
		mon = 12;
	
	t[2] = 0;
	sw_build_date = atoi(year)*1000000 + mon*10000 + atoi(day)*100 + atoi(t);
}
static uint16_t fill_payload(uint16_t msg_id, uint8_t *cmd, uint16_t cmd_len,
		uint8_t *rsp)
{
	uint16_t payload_len = 0;
	/* fill reserved */
	rsp[0] = 0x00;
	rsp[1] = 0x00;
	rsp[2] = 0x00;
	rsp[3] = 0x00;
	switch (msg_id) {
		case MSG_ID_HW_VER:
		case MSG_ID_SW_VER:
			rsp[6] = MCU_ERR_SUCCESS; 
			if (msg_id == MSG_ID_SW_VER) {
				get_sw_ver();
				rsp[8] = (sw_major_version >> 8) & 0xff;
				rsp[7] = sw_major_version & 0xff;
				rsp[10] = (sw_minor_version >> 8) & 0xff;
				rsp[9] = sw_minor_version & 0xff;
				rsp[12] = (sw_patch_version >> 8) & 0xff;
				rsp[11] = sw_patch_version & 0xff;
				rsp[16] = (sw_build_date >> 24) & 0xff;
				rsp[15] = (sw_build_date >> 16) & 0xff;
				rsp[14] = (sw_build_date >> 8) & 0xff;
				rsp[13] = (sw_build_date >> 0) & 0xff;
			} else {
				rsp[8] = (hw_major_version >> 8) & 0xff;
				rsp[7] = hw_major_version & 0xff;
				rsp[10] = (hw_minor_version >> 8) & 0xff;
				rsp[9] = hw_minor_version & 0xff;
				rsp[12] = (hw_patch_version >> 8) & 0xff;
				rsp[11] = hw_patch_version & 0xff;
				rsp[16] = (hw_build_date >> 24) & 0xff;
				rsp[15] = (hw_build_date >> 16) & 0xff;
				rsp[14] = (hw_build_date >> 8) & 0xff;
				rsp[13] = (hw_build_date >> 0) & 0xff;
			}
			payload_len = 11;
			break;
		case HEART_CMD:
			rsp[6] = MCU_ERR_SUCCESS;
			rt_memcpy(rsp+7, heart_ts[0], 8);
			rt_memcpy(rsp+15, heart_ts[1], 8);
			read_ts_64(rsp+23);
			payload_len = 25;
			break;
		case HOST_CMD_GET:
		case HOST_CMD_SET:
			payload_len = handle_cmd_id(msg_id, cmd, cmd_len, rsp+6);
			break;
		case DEVICE_EVENT_PSENSOR:
		case DEVICE_EVENT_KEY:
		case DEVICE_EVENT_MAG:
		case DEVICE_EVENT_VSYNC:
		case DEVICE_EVENT_TMP_HEAD:
		case DEVICE_EVENT_TMP_TAIL:
		case DEVICE_EVENT_ENV_LIGHT:
			payload_len = handle_event(msg_id, cmd, cmd_len, rsp+6);
			break;
		default:
			break;

	}
	rsp[5] = (payload_len >> 8) & 0xff;
	rsp[4] = payload_len & 0xff;

	return payload_len;
}

static void mcu_msg_handler(CyU3PDmaChannel *chHandle, uint8_t *msg)
{
	/* msg structure 
	   0   1   2 3     4 5          6 7 8 9  10..31
	   err h/d msg_id  payload_len  reserve  payload
	   */

	rt_bool_t status = RT_TRUE;    
	CyU3PDmaBuffer_t    out;
	uint8_t rsp[64] = {0};
	uint16_t rsp_msg_id = 0;
	uint16_t out_payload_len = 0;
	uint8_t err = msg[0];
	uint16_t cmd_id = 0;
	uint16_t msg_id = (msg[2] << 8) | msg[3];
	uint16_t cmd_len = (msg[4] << 8) | msg[5];
	uint8_t *cmd = (uint8_t *)(msg+10);
	uint32_t reserve = (msg[6] << 24) | (msg[7] << 16) |
		(msg[8] << 8) | msg[9];

	if (use_g_msg && msg[1] == HOST_CMD) {
		/* BUG with host long cmd */
		err = g_msg[0];
		msg_id = (g_msg[1] << 8) | g_msg[2];
		cmd_len = (g_msg[3] << 8) | g_msg[4];
		cmd = (uint8_t *)(g_msg + 9);
		reserve = (g_msg[5] << 24) | (g_msg[6] << 16) |
			(g_msg[7] << 8) | g_msg[8];
	}

	if (msg_id == HOST_CMD_GET || msg_id == HOST_CMD_SET)
		cmd_id = (cmd[2] << 8) | cmd[1];

	if (err == MCU_ERR_SUCCESS)
		dump_mcu_cmd(msg_id, cmd_id, cmd, cmd_len);
	else {
		nprintf("Error cmd %x from host\r\n", err);
		return;
	}

	//status = CyU3PDmaChannelGetBuffer (chHandle, &out, CYU3P_NO_WAIT);
	//if (status == CY_U3P_SUCCESS)
	//{
		//rsp = out.buffer;
		//CyU3PMemSet(out.buffer, 0, 512);
		/* msg 0 STX */
		rsp[0] = HOST_CMD_STX;
		/* msg 1 ts */
		read_ts_64(rsp+1);
		/* msg 9 msg_id */
		if (err == MCU_ERR_SUCCESS) {
			if (HOST_CMD == msg[1])
				rsp_msg_id = get_rsp_msg_id(msg_id);
			else
				rsp_msg_id = msg_id;
		} else {
			rsp_msg_id = 0xffff;
		}
		rsp[CMD_MSGID_1] = (rsp_msg_id >> 8) & 0xff;
		rsp[CMD_MSGID_0] = rsp_msg_id & 0xff;
		/* msg 11 reserve,payload len,payload */
		if (err == MCU_ERR_SUCCESS) {
			out_payload_len = fill_payload(msg_id, cmd, cmd_len,
					rsp+CMD_RESERVE_OFS);
		} else {
			rsp[CMD_PAYLOAD_LEN_0] = 0x01;
			rsp[CMD_PAYLOAD_LEN_1] = 0x00;
			rsp[CMD_PAYLOAD_OFS] = err;
			out_payload_len = 1;

		}
		/*int i;
		for (i=0; i<17+out_payload_len; i++)
			nprintf("-> %x\r\n",
					rsp[i]);*/
		/* encrypt reserve, payload len, payload */
		set_rc4_key(rsp[7]%10, rsp_msg_id, rsp+1);
		rc4(rsp+CMD_RESERVE_OFS,
				rsp+CMD_RESERVE_OFS, out_payload_len+2+4);
		/* msg n crc */
		uint16_t crc_ofs = 17 + out_payload_len;

		uint32_t crc = crc32((uint8_t *)rsp, crc_ofs);
		rsp[crc_ofs+3]   = (crc >> 24) & 0xff;
		rsp[crc_ofs+2] = (crc >> 16) & 0xff;
		rsp[crc_ofs+1] = (crc >> 8) & 0xff;
		rsp[crc_ofs+0] = (crc >> 0) & 0xff;
		if (!insert_mem(TYPE_D2H, rsp, crc_ofs+4))
			nprintf("lost d2h packet\r\n");
		//nprintf("begin host cmd ack %d\r\n",
		//			crc_ofs+4);
		//status = CyU3PDmaChannelCommitBuffer (chHandle, crc_ofs+4,
		//		CYU3P_NO_WAIT);
		//if (status != CY_U3P_SUCCESS)
		//{
		//	nprintf("host mcu cmd op failed %x\r\n",
		//			status);
		//	CyU3PDmaChannelReset (chHandle);
		//	CyU3PDmaChannelSetXfer (chHandle, 0);
		//}
	//} else {
	//	nprintf("host mcu cmd op getBuf failed %x\r\n",
	//			status);
		//CyU3PDmaChannelReset (chHandle);
		//CyU3PDmaChannelSetXfer (chHandle, 0);
	//	host_alive = 0;
	//}
}
