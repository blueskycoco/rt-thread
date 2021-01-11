#ifndef _INCLUDED_MCU_HID_H_
#define _INCLUDED_MCU_HID_H_

#include <cyu3types.h>
#include <cyu3os.h>
#include <cyu3usbconst.h>
#include <cyu3externcstart.h>

/* for imu data upload, MCU config write/read */

#define CY_MCU_HID_EP_INTR_IN	(0x86)          /* EP IN */
#define CY_MCU_HID_EP_INTR_OUT	(0x06)          /* EP OUT */

#define CY_MCU_HID_DMA_BUF_COUNT	(4)	/* DMA Buffer Count */

#define CY_MCU_APP_HOST_CMD_EVENT	(1 << 0)	/* Host Cmd Event Flag */

#define CY_MCU_HID_SET_IDLE	(0x0A)	/* HID Class Specific Setup Request */
#define CY_MCU_GET_REPORT_DESC	(0x22)	/* HID Standard Request */

#define CY_MCU_USB_HID_DESC_TYPE	(0x21)	/* HID Descriptor */

/* host cmd */
#define HOST_CMD 0
#define DEVICE_EVENT 1
#define HOST_CMD_GET 0x2000
#define HOST_CMD_GET_RSP 0x2001
#define HOST_CMD_SET 0x2002
#define HOST_CMD_SET_RSP 0x2003
#define HEART_CMD 0x2500
#define HEART_CMD_RSP 0x2500
#define DEVICE_EVENT_PSENSOR 0x1000
#define DEVICE_EVENT_KEY 0x1001
#define DEVICE_EVENT_MAG 0x1002
#define DEVICE_EVENT_VSYNC 0x1003
#define DEVICE_EVENT_TMP_HEAD 0x1005
#define DEVICE_EVENT_TMP_TAIL 0x1006
#define DEVICE_EVENT_ENV_LIGHT 0x1007
#define HOST_CMD_GET_HW_VER 0x0002
#define HOST_CMD_GET_SW_VER 0x0003



#define HOST_CMD_STX 0xfd
#define PAYLOAD_STATUS_SET_CMD	0x01
#define PAYLOAD_STATUS_SET_ACK	0x02
#define PAYLOAD_STATUS_GET_CMD	0x03
#define PAYLOAD_STATUS_GET_ACK	0x04
#define PAYLOAD_STATUS_SUPR_CMD	0x05
#define PAYLOAD_STATUS_SUPR_ACK	0x06
#define PAYLOAD_STATUS_EVENT	0x07
#define PAYLOAD_STATUS_TEST_CMD	0x08

#define MCU_ERR_SUCCESS	0x00
#define MCU_ERR_FAILE	0x01
#define MCU_ERR_INV_ARG 0x02
#define MCU_ERR_NO_MEM 0x03
#define MCU_ERR_UNSUPPORT 0x04
#define MCU_ERR_CRC	0x05
#define MCU_ERR_NREAL	0x06

#define MCU_BIN_STATE_DISABLE 0x00
#define MCU_BIN_STATE_ENABLE 0x01
#define MCU_BIN_STATE_PSENSOR_AWAY 0x01
#define MCU_BIN_STATE_PSENSOR_NEAR 0x00
#define MCU_BIN_KEY_UP	0x00
#define MCU_BIN_KEY_DOWN 0x01

#define MSG_ID_RC4_KEY	0x00
#define MSG_ID_HEART	0x01
#define MSG_ID_HW_VER	0x02
#define MSG_ID_SW_VER	0x03
#define MSG_ID_RESTART_DISP 0x05

#define MSG_ID_ADJUST_FLAGSET 0x20
#define MSG_ID_HAR_OLED1_ORBIT 0x21
#define MSG_ID_HAR_OLED2_ORBIT 0x22
#define MSG_ID_VER_OLED1_ORBIT 0x23
#define MSG_ID_VER_OLED2_ORBIT 0x24

#define MSG_ID_DISP_POWER_CTL	0x80
#define MSG_ID_LIGHT_CTL	0x81
#define MSG_ID_VSYNC_CTL 	0x82
#define MSG_ID_MAGIC_CTL	0x84
#define MSG_ID_RGB_LED_CTL	0x85
#define MSG_ID_BRIGHT_CTL	0x86
#define MSG_ID_BRIGHT_MAX_CTL	0x87
#define MSG_ID_DISP_MODE	0x88
#define MSG_ID_SPK_GAIN_CTL	0x89
#define MSG_ID_COLOR_CTL	0x8a
#define MSG_ID_DISP_DUTY_CTL	0x8b
#define MSG_ID_POWER_TIME_CTL	0x8c
#define MSG_ID_HOST_ID_CTL	0x8d
#define MSG_ID_OPER_ID_CTL	0x8e
#define MSG_ID_GLASS_ID_CTL	0x8f
#define MSG_ID_PSENS_NEAR_CTL	0x90
#define MSG_ID_PSENS_FAR_CTL	0x91
#define MSG_ID_7211_VER_CTL	0x92
#define MSG_ID_7211_UPDATE	0x93
#define MSG_ID_GLASSES_RESET	0x94
#define MSG_ID_GET_TEMP		0x95
#define MSG_ID_GET_DEVICE_NAME  0x96
#define MSG_ID_GET_CPU		0x97
#define MSG_ID_GET_ROM		0x98
#define MSG_ID_GET_RAM		0x99
#define MSG_ID_TEST_BRIGHT	0x9a

#define CMD_MSGID_0 9
#define CMD_MSGID_1 10
#define CMD_PAYLOAD_LEN_0 15
#define CMD_PAYLOAD_LEN_1 16
#define CMD_RESERVE_OFS 11
#define CMD_TS_OFS 1
#define CMD_LEN_PAYLOAD_OFS 15
#define CMD_PAYLOAD_OFS 17

enum {
	/* base 20200825 */
	NR_BRIGHTNESS = 1,
	NR_DISPLAY_2D_3D,
	NR_DISPLAY_VERSION,
	NR_HOST_ID,
	NR_GLASSID,
	NR_PSENSOR_CLOSED,
	NR_PSENSOR_NOCLOSED,
	NR_TEMPERATURE,
	NR_DISPLAY_DUTY,
	NR_POWER_FUCTION,
	NR_MAGNETIC_FUCTION,
	NR_VSYNC_FUCTION,
	NR_ENV_LIGHT,
	NR_WORLD_LED,
	NR_SLEEP_TIME,
	NR_7211_VERSION,
	NR_7211_UPDATE,
	NR_REBOOT,
	NR_BRIGHTNESS_EXT,
	NR_TEMPERATURE_FUNCTION,
	NR_TRY_CTRL_DISPLAY_STATUS,
	NR_TEMPERATURE_EXT,
	NR_BRIGHTNESS_MAX,
	NR_SPEAKER_LEVEL,
	NR_CPU_INFO,
	NR_ROM_INFO,
	NR_RAM_INFO,
	NR_LEFT_OLED_H_ORBIT,
	NR_LEFT_OLED_V_ORBIT,
	NR_RIGHT_OLED_H_ORBIT,
	NR_RIGHT_OLED_V_ORBIT,
	NR_ORBIT_ADJUST,
	NR_COLOR,
	NR_RECOVERY_FACTORY,
	NR_GLASS_BRI_TEST,
	NR_MACHINE_ID,
	NR_RGB_RESET,
	NR_MAX
};

enum {
	/* base 20200826 */
	NR_EVENT_PSENSOR = 0x1000,
	NR_EVENT_KEY,
	NR_EVENT_MAG,
	NR_EVENT_VSYNC,
	NR_EVENT_OTA_STS,
	NR_EVENT_TEMP_F,
	NR_EVENT_TEMP_T,
	NR_EVENT_ENV_LIGHT
};

typedef uint16_t (*mcu_func)( uint8_t *in, uint16_t in_len, uint8_t *out);
/* Extern definitions for the USB Enumeration descriptors */
#include <cyu3externcend.h>


void mcu_hid_init (void);
CyBool_t mcu_hid_class_rqt(uint16_t wIndex, uint8_t bRequest);
CyBool_t mcu_hid_stand_rqt(uint16_t wIndex, CyBool_t active);
void mcu_hid_stop (void);
void mcu_hid_start (void);
void mcu_hid_event_set(uint32_t event);
void build_event(uint8_t *msg, uint16_t msg_id, uint16_t len);
CyU3PReturnStatus_t mcu_msg_send (
        uint32_t *msg,
        uint32_t wait,
        CyBool_t pri);
uint32_t mcu_event_get();
void mcu_event_handler(uint32_t ev_stat);
void handle_event_host_cmd();
void get_g_param();
/*[]*/
#endif /* _INCLUDED_MCU_HID_H_ */

