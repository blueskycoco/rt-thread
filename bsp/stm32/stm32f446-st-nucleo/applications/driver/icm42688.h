/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-09-03     Ernest Chen  the first version
 */

#ifndef __ICM42688_H__
#define __ICM42688_H__

#include <rthw.h>
#include <rtthread.h>

enum icm42688_set_cmd
{
    ICM42688_PWR_MGMT1,     // power management 1
    ICM42688_PWR_MGMT2,     // power management 2
    ICM42688_GYRO_CONFIG,   // gyroscope configuration(range)
    ICM42688_ACCEL_CONFIG, // accelerometer configuration(range)
    ICM42688_ACCEL_CONFIG2, // accelerometer configuration2
    ICM42688_INT_ENABLE,    //interrupt enable
};
typedef enum icm42688_set_cmd icm42688_set_cmd_t;

enum icm42688_gyroscope_range
{
    ICM42688_GYROSCOPE_RANGE0, // ¡À250dps
    ICM42688_GYROSCOPE_RANGE1, // ¡À500dps
    ICM42688_GYROSCOPE_RANGE2, // ¡À1000dps
    ICM42688_GYROSCOPE_RANGE3, // ¡À2000dps
};
typedef enum icm42688_gyroscope_range icm42688_gyro_range_t;

enum icm42688_accelerometer_range
{
    ICM42688_ACCELEROMETER_RANGE0, // ¡À2g
    ICM42688_ACCELEROMETER_RANGE1, // ¡À4g
    ICM42688_ACCELEROMETER_RANGE2, // ¡À8g
    ICM42688_ACCELEROMETER_RANGE3, // ¡À16g
};
typedef enum icm42688_accelerometer_range icm42688_accel_range_t;

typedef struct cm42688_axes
{
    rt_int16_t x;
    rt_int16_t y;
    rt_int16_t z;
} cm42688_axes_t;

struct icm42688_device
{
    struct rt_spi_device *spi;
    cm42688_axes_t accel_offset;
    cm42688_axes_t gyro_offset;
    rt_mutex_t lock;
};
typedef struct icm42688_device *icm42688_device_t;

/**
 * This function initializes icm42688 registered device driver
 *
 * @param dev the name of icm42688 device
 *
 * @return the icm42688 device.
 */
icm42688_device_t icm42688_init();

/**
 * This function releases memory and deletes mutex lock
 *
 * @param dev the pointer of device driver structure.
 */
void icm42688_deinit(icm42688_device_t dev);

/**
 * This function gets accelerometer by icm42688 sensor measurement
 *
 * @param dev the pointer of device driver structure
 * @param accel_x the accelerometer of x-axis digital output
 * @param accel_y the accelerometer of y-axis digital output
 * @param accel_z the accelerometer of z-axis digital output
 *
 * @return the getting status of accelerometer, RT_EOK reprensents getting accelerometer successfully.
 */
rt_err_t icm42688_get_accel(icm42688_device_t dev, rt_int16_t *accel_x,
		rt_int16_t *accel_y, rt_int16_t *accel_z, rt_int16_t *gyro_x,
		rt_int16_t *gyro_y, rt_int16_t *gyro_z);

/**
 * This function gets gyroscope by icm42688 sensor measurement
 *
 * @param dev the pointer of device driver structure
 * @param gyro_x the gyroscope of x-axis digital output
 * @param gyro_y the gyroscope of y-axis digital output
 * @param gyro_z the gyroscope of z-axis digital output
 *
 * @return the getting status of gyroscope, RT_EOK reprensents getting gyroscope successfully.
 */
rt_err_t icm42688_get_gyro(icm42688_device_t dev, rt_int16_t *gyro_x, rt_int16_t *gyro_y, rt_int16_t *gyro_z);

/**
 * This function calibrates original gyroscope and accelerometer, which are bias, in calibration mode
 *
 * @param dev the pointer of device driver structure
 * @param times averaging times in calibration mode
 *
 * @return the getting original status, RT_EOK reprensents getting gyroscope successfully.
 */
rt_err_t icm42688_calib_level(icm42688_device_t dev, rt_size_t times);

/**
 * This function sets parameter of icm42688 sensor
 *
 * @param dev the pointer of device driver structure
 * @param cmd the parameter cmd of device
 * @param value for setting value in cmd register
 *
 * @return the setting parameter status, RT_EOK reprensents setting successfully.
 */
rt_err_t icm42688_set_param(icm42688_device_t dev, icm42688_set_cmd_t cmd, rt_uint8_t value);

/**
 * This function gets parameter of icm42688 sensor
 *
 * @param dev the pointer of device driver structure
 * @param cmd the parameter cmd of device
 * @param value to get value in cmd register
 *
 * @return the getting parameter status,RT_EOK reprensents getting successfully.
 */
rt_err_t icm42688_get_param(icm42688_device_t dev, icm42688_set_cmd_t cmd, rt_uint8_t *value);

#endif /*__DRV_ICM42688_H__ */
