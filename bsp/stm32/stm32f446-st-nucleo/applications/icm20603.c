#include <rtthread.h>
#include <rtdevice.h>
#include <string.h>
#include "board.h"
#include "drv_spi.h"
#include "icm20603.h"
#if defined(BSP_USING_ICM_20603)
/*
	nucleo-stm32f446ze      icm-20603
	
	PA4                     /CS
	PA5                     SCL_SCLK
	PA6                     AD0_SDO
	PA7                     SDA_SDI
	PB4                     INT
	3P3V                    VIN
	GND                     GND
 */

#ifndef RT_ICM20603_SPI_MAX_HZ
#define RT_ICM20603_SPI_MAX_HZ 8000000
#endif

#define RT_ICM20603_DEFAULT_SPI_CFG                  \
{                                                \
    .mode = RT_SPI_MODE_3 | RT_SPI_MSB,          \
    .data_width = 8,                             \
    .max_hz = RT_ICM20603_SPI_MAX_HZ,                \
}

#define DBG_ENABLE
#define DBG_SECTION_NAME "icm20603"
#define DBG_LEVEL DBG_LOG
#define DBG_COLOR
#include <rtdbg.h>

// Register map for icm20603
#define ICM20603_CONFIG_REG 0x1A        // configuration:fifo, ext sync and dlpf
#define ICM20603_GYRO_CONFIG_REG 0x1B   // gyroscope configuration
#define ICM20603_ACCEL_CONFIG1_REG 0x1C // accelerometer configuration
#define ICM20603_ACCEL_CONFIG2_REG 0x1D // accelerometer configuration
#define ICM20603_INT_ENABLE_REG 0x38    // interrupt enable
#define ICM20603_ACCEL_MEAS 0x3B        // accelerometer measurements
#define ICM20603_GYRO_MEAS 0x43         // gyroscope measurements
#define ICM20603_PWR_MGMT1_REG 0x6B     // power management 1
#define ICM20603_PWR_MGMT2_REG 0x6C     // power management 2

#define ICM20603_ADDR 0x68 /* slave address, ad0 set 0 */

/**
 * This function burst writes sequences, include single-byte.
 *
 * @param bus the pointer of iic bus
 * @param reg the address regisgter
 * @param len the length of bytes
 * @param buf the writing value
 *
 * @return the writing status of accelerometer, RT_EOK reprensents setting successfully.
 */
static rt_err_t write_regs(struct rt_spi_device *spi_dev, rt_uint8_t reg,
		rt_uint8_t len, rt_uint8_t *buf)
{
	rt_uint8_t tx[32] = {0};
	tx[0] = reg;
	memcpy(tx+1, buf, len);
	int ret = rt_spi_send(spi_dev, tx, len+1);
	return (ret == len+1) ? RT_EOK : RT_ERROR;
}

static rt_err_t read_regs(struct rt_spi_device *spi_dev, rt_uint8_t reg,
		rt_uint8_t len, rt_uint8_t *buf)
{
	int i;
	rt_uint8_t tx[32] = {0};
	rt_uint8_t rx[32] = {0};
	tx[0] = reg|0x80;
	rt_memset(tx+1, 0xff, len);
	rt_size_t ret = rt_spi_transfer(spi_dev, tx, rx, len+1);
	memcpy(buf, rx+1, len);
	return (ret == len+1) ? RT_EOK : RT_ERROR;
}

static rt_err_t read_regs_ext(struct rt_spi_device *spi_dev, rt_uint8_t reg,
		rt_uint8_t len, rt_uint8_t *buf)
{
	int i;
	rt_uint8_t tx[32] = {0};
	rt_uint8_t rx[32] = {0};
	tx[0] = reg|0x80;
	rt_memset(tx+1, 0xff, len);
	rt_size_t ret = rt_spi_transfer(spi_dev, tx, rx, len+1);
	memcpy(buf, rx, len);
	return (ret == len+1) ? RT_EOK : RT_ERROR;
}
static rt_err_t reset_sensor(icm20603_device_t dev)
{
    rt_uint8_t value = 0;

    RT_ASSERT(dev);

    return write_regs(dev->spi, ICM20603_PWR_MGMT1_REG, 1, &value);
}
void icm20603_int_enable(icm20603_device_t dev)
{
    icm20603_set_param(dev, ICM20603_INT_ENABLE, 1);
}
static rt_err_t icm20603_sensor_init(icm20603_device_t dev)
{
    rt_err_t result = -RT_ERROR;

    RT_ASSERT(dev);

    result = rt_mutex_take(dev->lock, RT_WAITING_FOREVER);
    if (result != RT_EOK)
    {
        LOG_E("This sensor initializes failure 1");
        goto __exit;
    }

    result = reset_sensor(dev);
    if (result != RT_EOK)
    {
        LOG_E("This sensor initializes failure 2");
        goto __exit;
    }
    rt_thread_mdelay(50);

    /* open 3 accelerometers and 3 gyroscope */
    result = icm20603_set_param(dev, ICM20603_PWR_MGMT2, 0);
    if (result != RT_EOK)
    {
        LOG_E("This sensor initializes failure3 ");
        goto __exit;
    }

    /* set gyroscope range, default 250 dps */
    result = icm20603_set_param(dev, ICM20603_GYRO_CONFIG, 3);
    if (result != RT_EOK)
    {
        LOG_E("This sensor initializes failure 4");
        goto __exit;
    }

    /* set accelerometer range, default 2g */
    result = icm20603_set_param(dev, ICM20603_ACCEL_CONFIG1, 3);
    if (result != RT_EOK)
    {
        LOG_E("This sensor initializes failure5");
        goto __exit;
    }

    /* ACCEL_FCHOICE_B = 0 and A_DLPF_CFG[2:0] = 1 */
    result = icm20603_set_param(dev, ICM20603_ACCEL_CONFIG2, 1);
    if (result != RT_EOK)
    {
        LOG_E("This sensor initializes failure6");
        goto __exit;
    }

    result = icm20603_set_param(dev, ICM20603_INT_ENABLE, 0);
    if (result != RT_EOK)
    {
        LOG_E("This sensor initializes failure6");
        goto __exit;
    }
__exit:
    if (result != RT_EOK)
    {
        LOG_E("This sensor initializes failure7");
    }
    rt_mutex_release(dev->lock);

    return result;
}

/**
 * This function calibrates original gyroscope and accelerometer, which are bias, in calibration mode
 *
 * @param dev the pointer of device driver structure
 * @param times averaging times in calibration mode
 *
 * @return the getting original status, RT_EOK reprensents getting gyroscope successfully.
 */
rt_err_t icm20603_calib_level(icm20603_device_t dev, rt_size_t times)
{
    rt_int32_t accel[3] = {0, 0, 0};
    rt_int32_t gyro[3] = {0, 0, 0};
    rt_size_t i;
    rt_err_t result;

    RT_ASSERT(dev);

    result = rt_mutex_take(dev->lock, RT_WAITING_FOREVER);
    if (result == RT_EOK)
    {
        for (i = 0; i < times; i++)
        {
            rt_int16_t x, y, z;

            /* read the sensor digital output */
            result = icm20603_get_accel(dev, &x, &y, &z);
            if (result == RT_EOK)
            {
                accel[0] += x;
                accel[1] += y;
                accel[2] += z;
            }
            else
            {
                break;
            }

            result = icm20603_get_gyro(dev, &x, &y, &z);
            if (result == RT_EOK)
            {
                gyro[0] += x;
                gyro[1] += y;
                gyro[2] += z;
            }
            else
            {
                break;
            }
        }

        if (result == RT_EOK)
        {
            dev->accel_offset.x = (accel[0] / (int)times);
            dev->accel_offset.y = (accel[1] / (int)times);
            dev->accel_offset.z = (accel[2] / (int)times) - 0x3fff; //default full range

            dev->gyro_offset.x = (int16_t)(gyro[0] / (int)times);
            dev->gyro_offset.y = (int16_t)(gyro[1] / (int)times);
            dev->gyro_offset.z = (int16_t)(gyro[2] / (int)times);
        }
    }

    if (result == RT_EOK)
    {
        rt_mutex_release(dev->lock);
    }
    else
    {
        LOG_E("Can't calibrate the sensor");
    }

    return result;
}


icm20603_device_t icm20603_init(void)
{
	icm20603_device_t dev;
	struct rt_spi_configuration cfg = RT_ICM20603_DEFAULT_SPI_CFG;

	__HAL_RCC_GPIOA_CLK_ENABLE();
	rt_hw_spi_device_attach("spi1", "spi10", GPIOA, GPIO_PIN_4);


	dev = rt_calloc(1, sizeof(struct icm20603_device));
	if (dev == RT_NULL)
	{
		LOG_E("Can't allocate memory for icm20603 device on 'spi10' ");
		rt_free(dev);

		return RT_NULL;
	}

	dev->spi = (struct rt_spi_device *)rt_device_find("spi10");

	if (dev->spi == RT_NULL)
	{
		LOG_E("Can't find icm20603 device on 'spi10'");
		rt_free(dev);

		return RT_NULL;
	}

	rt_spi_configure(dev->spi, &cfg);

	dev->lock = rt_mutex_create("mutex_icm20603", RT_IPC_FLAG_FIFO);
	if (dev->lock == RT_NULL)
	{
		LOG_E("Can't create mutex for icm20603 device on 'spi10'");
		rt_free(dev);

		return RT_NULL;
	}

	/* init icm20603 sensor */
	if (icm20603_sensor_init(dev) != RT_EOK)
	{
		LOG_E("Can't init icm20603 device on 'spi10'");
		rt_free(dev);
		rt_mutex_delete(dev->lock);

		return RT_NULL;
	}

	return dev;
}

/**
 * This function releases memory and deletes mutex lock
 *
 * @param dev the pointer of device driver structure
 */
void icm20603_deinit(icm20603_device_t dev)
{
    RT_ASSERT(dev);

    rt_mutex_delete(dev->lock);
    rt_free(dev);
}

/**
 * This function gets accelerometer by icm20603 sensor measurement
 *
 * @param dev the pointer of device driver structure
 * @param accel_x the accelerometer of x-axis digital output
 * @param accel_y the accelerometer of y-axis digital output
 * @param accel_z the accelerometer of z-axis digital output
 *
 * @return the getting status of accelerometer, RT_EOK reprensents  getting accelerometer successfully.
 */
rt_err_t icm20603_get_accel(icm20603_device_t dev, rt_int16_t *accel_x,
		rt_int16_t *accel_y, rt_int16_t *accel_z)
{
    rt_err_t result = -RT_ERROR;
    rt_uint8_t value[6];
    rt_uint8_t range = 0;

    RT_ASSERT(dev);

    result = rt_mutex_take(dev->lock, RT_WAITING_FOREVER);
    if (result == RT_EOK)
    {
        result = icm20603_get_param(dev, ICM20603_ACCEL_CONFIG1, &range); // default 2g

        if (range < 4 && result == RT_EOK)
        {
            result = read_regs(dev->spi, ICM20603_ACCEL_MEAS, 6, &value[0]); //self test x,y,z accelerometer;

            if (result != RT_EOK)
            {
                LOG_E("Failed to get accelerometer value of icm20603");
            }
            else
            {
                *accel_x = (value[0] << 8) + value[1];// - dev->accel_offset.x;
                *accel_y = (value[2] << 8) + value[3];// - dev->accel_offset.y;
                *accel_z = (value[4] << 8) + value[5];// - dev->accel_offset.z;
            }
        }
        else
        {
            LOG_E("Failed to get accelerometer value of icm20603");
        }

        rt_mutex_release(dev->lock);
    }
    else
    {
        LOG_E("Failed to get accelerometer value of icm20603");
    }

    return result;
}

/**
 * This function gets gyroscope by icm20603 sensor measurement
 *
 * @param dev the pointer of device driver structure
 * @param gyro_x the gyroscope of x-axis digital output
 * @param gyro_y the gyroscope of y-axis digital output
 * @param gyro_z the gyroscope of z-axis digital output
 *
 * @return the getting status of gyroscope, RT_EOK reprensents getting gyroscope successfully.
 */
rt_err_t icm20603_get_gyro(icm20603_device_t dev, rt_int16_t *gyro_x,
		rt_int16_t *gyro_y, rt_int16_t *gyro_z)
{
    rt_err_t result = -RT_ERROR;
    rt_uint8_t range = 0;
    rt_uint8_t value[6];

    RT_ASSERT(dev);

    result = rt_mutex_take(dev->lock, RT_WAITING_FOREVER);
    if (result == RT_EOK)
    {
        result = icm20603_get_param(dev, ICM20603_GYRO_CONFIG, &range); // default 250dps

        if (range < 4 && result == RT_EOK)
        {
            result = read_regs(dev->spi, ICM20603_GYRO_MEAS, 6, &value[0]); //self test x,y,z gyroscope;

            if (result != RT_EOK)
            {
                LOG_E("Failed to get gyroscope value of icm20603");
            }
            else
            {
                *gyro_x = (value[0] << 8) + value[1];// - dev->gyro_offset.x;
                *gyro_y = (value[2] << 8) + value[3];// - dev->gyro_offset.y;
                *gyro_z = (value[4] << 8) + value[5];// - dev->gyro_offset.z;
            }
        }
        else
        {
            LOG_E("Failed to get gyroscope value of icm20603");
        }

        rt_mutex_release(dev->lock);
    }
    else
    {
        LOG_E("Failed to get gyroscope value of icm20603");
    }

    return result;
}

/**
 * This function sets parameter of icm20603 sensor
 *
 * @param dev the pointer of device driver structure
 * @param cmd the parameter cmd of device
 * @param value for setting value in cmd register
 *
 * @return the setting parameter status,RT_EOK reprensents setting successfully.
 */
rt_err_t icm20603_set_param(icm20603_device_t dev, icm20603_set_cmd_t cmd,
		uint8_t value)
{
    rt_err_t result = -RT_ERROR;
    RT_ASSERT(dev);

    switch (cmd)
    {
    case ICM20603_GYRO_CONFIG:
    {
        rt_uint8_t args;

        if (!(value == ICM20603_GYROSCOPE_RANGE0 ||
        	value == ICM20603_GYROSCOPE_RANGE1 ||
        	value == ICM20603_GYROSCOPE_RANGE2 ||
        	value == ICM20603_GYROSCOPE_RANGE3))
        {
            LOG_E("Setting gyroscope range is wrong, please refer gyroscope range");

            return -RT_ERROR;
        }

        result = read_regs(dev->spi, ICM20603_GYRO_CONFIG_REG, 1, &args);
        if (result == RT_EOK)
        {
        	rt_kprintf("gyro args %x\r\n", args);
            args &= 0xE7; //clear [4:3]
            args |= value << 3;
            result = write_regs(dev->spi, ICM20603_GYRO_CONFIG_REG, 1, &args);
        }

        break;
    }
    case ICM20603_ACCEL_CONFIG1:
    {
        rt_uint8_t args;

        if (!(value == ICM20603_ACCELEROMETER_RANGE0 ||
        		value == ICM20603_ACCELEROMETER_RANGE1 ||
        		value == ICM20603_ACCELEROMETER_RANGE2 ||
        		value == ICM20603_ACCELEROMETER_RANGE3))
        {
            LOG_E("Setting als accelerometer range is wrong, please refer accelerometer range");

            return -RT_ERROR;
        }

        result = read_regs(dev->spi, ICM20603_ACCEL_CONFIG1_REG, 1, &args);
        if (result == RT_EOK)
        {
        	rt_kprintf("accel args %x\r\n", args);
            args &= 0xE7; //clear [4:3]
            args |= value << 3;
            result = write_regs(dev->spi, ICM20603_ACCEL_CONFIG1_REG, 1, &args);
        }
        break;
    }
    case ICM20603_ACCEL_CONFIG2:
    {
        result = write_regs(dev->spi, ICM20603_ACCEL_CONFIG2_REG, 1, &value);
        break;
    }
    case ICM20603_PWR_MGMT2:
    {
        result = write_regs(dev->spi, ICM20603_PWR_MGMT2_REG, 1, &value);
        break;
    }
    case ICM20603_INT_ENABLE:
    {
    	    rt_uint8_t tmp = 0x01;
        result = write_regs(dev->spi, 0x1a, 1, &tmp);
        	tmp = 0x00;
        result = write_regs(dev->spi, 0x19, 1, &tmp);
        result = write_regs(dev->spi, ICM20603_INT_ENABLE_REG, 1, &value);
        break;
    }

    default:
    {
        LOG_E("This cmd'%2x' can not be set or supported", cmd);

        return -RT_ERROR;
    }
    }

    return result;
}

/**
 * This function gets parameter of icm20603 sensor
 *
 * @param dev the pointer of device driver structure
 * @param cmd the parameter cmd of device
 * @param value to get value in cmd register
 *
 * @return the getting parameter status,RT_EOK reprensents getting successfully.
 */
rt_err_t icm20603_get_param(icm20603_device_t dev, icm20603_set_cmd_t cmd,
			rt_uint8_t *value)
{
    rt_err_t result = -RT_ERROR;

    RT_ASSERT(dev);

    switch (cmd)
    {
    case ICM20603_GYRO_CONFIG:
    {
        rt_uint8_t args;

        result = read_regs(dev->spi, ICM20603_GYRO_CONFIG_REG, 1, &args);
        *value = (args >> 3) & 0x3;
        break;
    }
    case ICM20603_ACCEL_CONFIG1:
    {
        rt_uint8_t args;

        result = read_regs(dev->spi, ICM20603_ACCEL_CONFIG1_REG, 1, &args);
        *value = (args >> 3) & 0x3;
        break;
    }
    case ICM20603_ACCEL_CONFIG2:
    {
        rt_uint8_t args;

        result = read_regs(dev->spi, ICM20603_ACCEL_CONFIG2_REG, 1, &args);
        break;
    }

    case ICM20603_PWR_MGMT2:
    {
        result = read_regs(dev->spi, ICM20603_PWR_MGMT2_REG, 1, value);
        break;
    }
    case ICM20603_INT_ENABLE:
    {
        result = read_regs(dev->spi, ICM20603_INT_ENABLE_REG, 1, value);
        break;
    }
    default:
    {
        LOG_E("This cmd'%2x' can not be get or supported", cmd);

        return -RT_ERROR;
    }
    }

    return result;
}
#endif
