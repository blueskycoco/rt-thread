/*
 * File      : led.h
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2009, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2009-01-05     Bernard      the first version
 */

#ifndef __LED_H__
#define __LED_H__

#include <rtthread.h>
#define TYPE_DGRAM_REGISTER				"1"
#define TYPE_DGRAM_DATA					"2"
#define TYPE_DGRAM_RE_DATA				"3"
#define TYPE_DGRAM_WARNING				"4"
#define TYPE_DGRAM_SYNC					"5"
#define TYPE_DGRAM_ASK_RE_DATA			"6"
#define TYPE_DEVICE_TRANS_WIFI			"2"
#define TYPE_DEVICE_TRANS_GPRS			"1"
#define TYPE_DEVICE_TRANS_3G			"3"
#define TYPE_DEVICE_TRANS_4G			"4"

#define ID_DGRAM_TYPE					"0"
#define ID_DEVICE_UID					"30"
#define ID_DEVICE_TYPE					"31"
#define ID_DEVICE_SUB_TYPE				"32"
#define ID_DEVICE_SW_VERSION			"33"
#define ID_DEVICE_TRANS_TYPE			"34"
#define ID_DEVICE_IP_ADDR				"35"
#define ID_DEVICE_PORT					"36"
#define ID_RE_DGRAM_TYPE				"38"

#define ID_DEVICE_CAP_TIME				"103"
#define ID_CAP_CO						"60"
#define ID_CAP_CO2						"61"
#define ID_CAP_HCHO						"62"
#define ID_CAP_TEMPERATURE				"63"
#define ID_CAP_SHI_DU					"64"
#define ID_CAP_PM_25					"65"
#define ID_CAP_PM_10					"66"
#define ID_CAP_BUZZY					"67"
#define ID_CAP_FENG_SU					"68"
#define ID_CAP_QI_YA					"69"
#define ID_CAP_CHOU_YANG				"70"
#define ID_CAP_SO2						"71"
#define ID_CAP_DONG_QI					"72"
#define ID_CAP_ZI_WAI					"73"
#define ID_CAP_TVOC						"74"
#define ID_CAP_BEN						"75"
#define ID_CAP_JIA_BEN					"76"
#define ID_CAP_2_JIA_BEN				"77"
#define ID_CAP_AN_QI					"78"
#define ID_CAP_HS						"79"
#define ID_CAP_NO						"160"
#define ID_CAP_NO2						"161"
#define ID_CAP_ZHAO_DU					"163"
#define ID_CAP_WEI_S_WU					"164"

#define ID_ALERT_OFF					"90"
#define ID_ALERT_SWITCH_CHANNEL			"91"
#define ID_ALERT_CAP_FAILED				"92"
#define ID_ALERT_NO_ACK					"93"
#define ID_ALERT_POWER_OFF				"94"
#define ID_RE_START_TIME				"101"
#define ID_RE_STOP_TIME					"102"
#define ID_CAP_TIME						"103"
#define ID_SERVER_TIME					"104"
#define ID_USER_LOCAL_PLACE				"200"
#define ID_USER_NAME					"201"
#define ID_USER_PHONE					"202"
#define ID_USER_ID						"203"
#define ID_DEVICE_SETUP_TIME			"210"
#define ID_DEVICE_SETUP_PLACE			"211"
#define ID_DEVICE_INFO					"212"
#define ID_DEVICE_STATUS				"213"

void rt_hw_led_init(void);
void rt_hw_led_on(rt_uint32_t led);
void rt_hw_led_off(rt_uint32_t led);

#endif
