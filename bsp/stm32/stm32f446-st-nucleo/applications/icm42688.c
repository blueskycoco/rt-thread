#include <rtthread.h>
#include <rtdevice.h>
#include <string.h>
#include "board.h"
#include "drv_spi.h"
#include "icm42688.h"
#if defined(BSP_USING_ICM_42688)
/*
	nucleo-stm32f446ze      icm-42688
	
	PE4                     /CS
	PE2                     SCL
	PE5                     SDO
	PE6                     SDA
	nul                     INT
	3P3V                    VIN
	GND                     GND
 */

#ifndef RT_ICM42688_SPI_MAX_HZ
#define RT_ICM42688_SPI_MAX_HZ 24000000
#endif

#define RT_ICM42688_DEFAULT_SPI_CFG                  \
{                                                \
    .mode = RT_SPI_MODE_3 | RT_SPI_MSB,          \
    .data_width = 8,                             \
    .max_hz = RT_ICM42688_SPI_MAX_HZ,                \
}

#define DBG_ENABLE
#define DBG_SECTION_NAME "icm42688"
#define DBG_LEVEL DBG_LOG
#define DBG_COLOR
#include <rtdbg.h>

// Register map for icm42688
#define ICM42688_CONFIG_REG 0x1A        // configuration:fifo, ext sync and dlpf
#define ICM42688_GYRO_CONFIG_REG 0x4f   // gyroscope configuration
#define ICM42688_ACCEL_CONFIG1_REG 0x50 // accelerometer configuration
#define ICM42688_ACCEL_CONFIG2_REG 0x1D // accelerometer configuration
#define ICM42688_INT_ENABLE_REG 0x38    // interrupt enable
#define ICM42688_ACCEL_MEAS 0x1f        // accelerometer measurements
#define ICM42688_GYRO_MEAS 0x25         // gyroscope measurements
#define ICM42688_DEV_CONFIG_REG 0x11     // power management 1
#define ICM42688_PWR_MGMT2_REG 0x4e     // power management 2
#define ICM42688_BANK_SEL	0x76
#define ICM42688_ADDR 0x68 /* slave address, ad0 set 0 */

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
static rt_err_t reset_sensor(icm42688_device_t dev)
{
    rt_uint8_t value = 1;
    rt_uint8_t bank = 0;

    RT_ASSERT(dev);
    
    write_regs(dev->spi, ICM42688_BANK_SEL, 1, &bank);
    
    return write_regs(dev->spi, ICM42688_DEV_CONFIG_REG, 1, &value);
}
static rt_err_t icm42688_sensor_init(icm42688_device_t dev)
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

    /* set gyroscope range, default 250 dps */
    result = icm42688_set_param(dev, ICM42688_GYRO_CONFIG, 6);
    if (result != RT_EOK)
    {
        LOG_E("This sensor initializes failure 4");
        goto __exit;
    }

    /* set accelerometer range, default 2g */
    result = icm42688_set_param(dev, ICM42688_ACCEL_CONFIG, 6);
    if (result != RT_EOK)
    {
        LOG_E("This sensor initializes failure5");
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

icm42688_device_t icm42688_init(void)
{
	icm42688_device_t dev;
	struct rt_spi_configuration cfg = RT_ICM42688_DEFAULT_SPI_CFG;
	rt_kprintf("to attach spi4\r\n");
	__HAL_RCC_GPIOE_CLK_ENABLE();
	rt_hw_spi_device_attach("spi4", "spi41", GPIOE, GPIO_PIN_4);


	dev = rt_calloc(1, sizeof(struct icm42688_device));
	if (dev == RT_NULL)
	{
		LOG_E("Can't allocate memory for icm42688 device on 'spi10' ");
		rt_free(dev);

		return RT_NULL;
	}

	dev->spi = (struct rt_spi_device *)rt_device_find("spi41");

	if (dev->spi == RT_NULL)
	{
		LOG_E("Can't find icm42688 device on 'spi41'");
		rt_free(dev);

		return RT_NULL;
	}

	rt_spi_configure(dev->spi, &cfg);

	dev->lock = rt_mutex_create("mutex_icm42688", RT_IPC_FLAG_FIFO);
	if (dev->lock == RT_NULL)
	{
		LOG_E("Can't create mutex for icm42688 device on 'spi10'");
		rt_free(dev);

		return RT_NULL;
	}

	/* init icm42688 sensor */
	if (icm42688_sensor_init(dev) != RT_EOK)
	{
		LOG_E("Can't init icm42688 device on 'spi41'");
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
void icm42688_deinit(icm42688_device_t dev)
{
    RT_ASSERT(dev);

    rt_mutex_delete(dev->lock);
    rt_free(dev);
}
rt_uint8_t icm42688_int_status(icm42688_device_t dev)
{
	rt_uint8_t value;
        rt_uint8_t bank = 0;
    	
    	write_regs(dev->spi, ICM42688_BANK_SEL, 1, &bank);
	read_regs(dev->spi, 0x2d, 1, &value);
	return value;
}
/**
 * This function gets accelerometer by icm42688 sensor measurement
 *
 * @param dev the pointer of device driver structure
 * @param accel_x the accelerometer of x-axis digital output
 * @param accel_y the accelerometer of y-axis digital output
 * @param accel_z the accelerometer of z-axis digital output
 *
 * @return the getting status of accelerometer, RT_EOK reprensents  getting accelerometer successfully.
 */
rt_err_t icm42688_get_accel(icm42688_device_t dev, rt_int16_t *accel_x,
		rt_int16_t *accel_y, rt_int16_t *accel_z, rt_int16_t *gyro_x,
		rt_int16_t *gyro_y, rt_int16_t *gyro_z)
{
    rt_err_t result = -RT_ERROR;
    rt_uint8_t value[14];
    rt_uint8_t range = 0;
    rt_uint8_t bank = 0;

    RT_ASSERT(dev);

    result = rt_mutex_take(dev->lock, RT_WAITING_FOREVER);
    if (result == RT_EOK)
    {
    	    write_regs(dev->spi, ICM42688_BANK_SEL, 1, &bank);
            result = read_regs(dev->spi, ICM42688_ACCEL_MEAS, 12, &value[0]); //self test x,y,z accelerometer;

            if (result != RT_EOK)
            {
                LOG_E("Failed to get accelerometer value of icm42688");
            }
            else
            {
                *accel_x = (value[0] << 8) + value[1];// - dev->accel_offset.x;
                *accel_y = (value[2] << 8) + value[3];// - dev->accel_offset.y;
                *accel_z = (value[4] << 8) + value[5];// - dev->accel_offset.z;
                *gyro_x = (value[6] << 8) + value[7];// - dev->accel_offset.x;
                *gyro_y = (value[8] << 8) + value[9];// - dev->accel_offset.y;
                *gyro_z = (value[10] << 8) + value[11];// - dev->accel_offset.z;
            }

        rt_mutex_release(dev->lock);
    }
    else
    {
        LOG_E("Failed to get accelerometer value of icm42688");
    }

    return result;
}

/**
 * This function sets parameter of icm42688 sensor
 *
 * @param dev the pointer of device driver structure
 * @param cmd the parameter cmd of device
 * @param value for setting value in cmd register
 *
 * @return the setting parameter status,RT_EOK reprensents setting successfully.
 */
rt_err_t icm42688_set_param(icm42688_device_t dev, icm42688_set_cmd_t cmd,
		uint8_t value)
{
    rt_err_t result = -RT_ERROR;
    RT_ASSERT(dev);

    switch (cmd)
    {
    case ICM42688_GYRO_CONFIG:
    {
        rt_uint8_t args;
	rt_uint8_t bank = 4;
	rt_uint8_t tmp;

    	//write_regs(dev->spi, ICM42688_BANK_SEL, 1, &bank);
    	//tmp = 0x0d;
    	//write_regs(dev->spi, 0x10, 1, &tmp);
    	//tmp = 0x01;
    	//write_regs(dev->spi, 0x12, 1, &tmp);
       	bank = 0;
    	write_regs(dev->spi, ICM42688_BANK_SEL, 1, &bank);
	read_regs(dev->spi, ICM42688_PWR_MGMT2_REG, 1, &args);
	args |= (0x03 << 2);
	write_regs(dev->spi, ICM42688_PWR_MGMT2_REG, 1, &args);	
    	//write_regs(dev->spi, ICM42688_BANK_SEL, 1, &bank);
    	//tmp = 0x2a;
    	//write_regs(dev->spi, 0x10, 1, &tmp);
       
       	bank = 0;
    	write_regs(dev->spi, ICM42688_BANK_SEL, 1, &bank);
        result = read_regs(dev->spi, ICM42688_GYRO_CONFIG_REG, 1, &args);
        if (result == RT_EOK)
        {
            rt_kprintf("gyro args %x\r\n", args);
            args &= 0x00; //clear [4:3]
            args |= value;
            result = write_regs(dev->spi, ICM42688_GYRO_CONFIG_REG, 1, &args);
        }

        break;
    }
    case ICM42688_ACCEL_CONFIG:
    {
        rt_uint8_t args;
	rt_uint8_t bank = 0;

    	write_regs(dev->spi, ICM42688_BANK_SEL, 1, &bank);
	read_regs(dev->spi, ICM42688_PWR_MGMT2_REG, 1, &args);
	args |= 0x03;
	write_regs(dev->spi, ICM42688_PWR_MGMT2_REG, 1, &args);	
        result = read_regs(dev->spi, ICM42688_ACCEL_CONFIG1_REG, 1, &args);
        if (result == RT_EOK)
        {
            rt_kprintf("accel args %x\r\n", args);
            args &= 0x00; //clear [4:3]
            args |= value;
            result = write_regs(dev->spi, ICM42688_ACCEL_CONFIG1_REG, 1, &args);
        }
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
#endif
