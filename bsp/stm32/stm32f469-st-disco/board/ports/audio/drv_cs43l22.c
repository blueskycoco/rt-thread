/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-11-14     ZeroFree     first implementation
 * 2019-07-28     Ernest       perfect player, add record funciton and other APIs
 */
#include <rtthread.h>
#include <rtdevice.h>
#include "drv_cs43l22.h"
#include <stdlib.h>

#define DBG_TAG              "drv.codec"
#define DBG_LVL              DBG_LOG
#include <rtdbg.h>
#define VOLUME_CONVERT(Volume)    (((Volume) > 100)? 255:((uint8_t)(((Volume) * 255) / 100)))  
/* Uncomment this line to enable verifying data sent to codec after each write 
   operation (for debug purpose) */
#if !defined (VERIFY_WRITTENDATA)  
/* #define VERIFY_WRITTENDATA */
#endif /* VERIFY_WRITTENDATA */
/* Audio codec driver structure initialization */  
#define CODEC_STANDARD                0x04
static uint8_t Is_cs43l22_Stop = 1;

volatile uint8_t OutputDev = 0;

/**
  * @}
  */ 

/** @defgroup CS43L22_Function_Prototypes
  * @{
  */
static uint8_t CODEC_IO_Write(struct rt_i2c_bus_device *dev, uint8_t Addr,
		uint8_t Reg, uint8_t Value);
static uint8_t CODEC_IO_Read(struct rt_i2c_bus_device *dev, uint8_t Addr,
		uint8_t Reg);
/**
  * @}
  */ 

/** @defgroup CS43L22_Private_Functions
  * @{
  */ 

/**
  * @brief Initializes the audio codec and the control interface.
  * @param DeviceAddr: Device address on communication Bus.   
  * @param OutputDevice: can be OUTPUT_DEVICE_SPEAKER, OUTPUT_DEVICE_HEADPHONE,
  *                       OUTPUT_DEVICE_BOTH or OUTPUT_DEVICE_AUTO .
  * @param Volume: Initial volume level (from 0 (Mute) to 100 (Max))
  * @retval 0 if correct communication, else wrong communication
  */
uint32_t cs43l22_Init(struct rt_i2c_bus_device *dev, uint16_t DeviceAddr,
		uint16_t OutputDevice, uint8_t Volume, uint32_t AudioFreq)
{
  uint32_t counter = 0;
 
  cs43l22_ReadID(dev, DeviceAddr);
	LOG_D("%s %d", __func__, __LINE__);
  /* Keep Codec powered OFF */
  counter += CODEC_IO_Write(dev, DeviceAddr, CS43L22_REG_POWER_CTL1, 0x01);  
  
  /*Save Output device for mute ON/OFF procedure*/
  switch (OutputDevice)
  {
  case OUTPUT_DEVICE_SPEAKER:
    OutputDev = 0xFA;
    break;
    
  case OUTPUT_DEVICE_HEADPHONE:
    OutputDev = 0xAF;
    break;
    
  case OUTPUT_DEVICE_BOTH:
    OutputDev = 0xAA;
    break;
    
  case OUTPUT_DEVICE_AUTO:
    OutputDev = 0x05;
    break;    
    
  default:
    OutputDev = 0x05;
    break;    
  }
  
  counter += CODEC_IO_Write(dev, DeviceAddr, CS43L22_REG_POWER_CTL2, OutputDev);
  
  /* Clock configuration: Auto detection */  
  counter += CODEC_IO_Write(dev, DeviceAddr, CS43L22_REG_CLOCKING_CTL, 0x81);
  
  /* Set the Slave Mode and the audio Standard */  
  counter += CODEC_IO_Write(dev, DeviceAddr, CS43L22_REG_INTERFACE_CTL1, CODEC_STANDARD);
  
  /* Set the Master volume */
  counter += cs43l22_SetVolume(dev, DeviceAddr, Volume);
  
  /* If the Speaker is enabled, set the Mono mode and volume attenuation level */
  if(OutputDevice != OUTPUT_DEVICE_HEADPHONE)
  {
	LOG_D("%s %d", __func__, __LINE__);
    /* Set the Speaker Mono mode */  
    counter += CODEC_IO_Write(dev, DeviceAddr, CS43L22_REG_PLAYBACK_CTL2, 0x06);
    
    /* Set the Speaker attenuation level */  
    counter += CODEC_IO_Write(dev, DeviceAddr, CS43L22_REG_SPEAKER_A_VOL, 0x00);
    counter += CODEC_IO_Write(dev, DeviceAddr, CS43L22_REG_SPEAKER_B_VOL, 0x00);
  }
  
  /* Additional configuration for the CODEC. These configurations are done to reduce
  the time needed for the Codec to power off. If these configurations are removed, 
  then a long delay should be added between powering off the Codec and switching 
  off the I2S peripheral MCLK clock (which is the operating clock for Codec).
  If this delay is not inserted, then the codec will not shut down properly and
  it results in high noise after shut down. */
  
  /* Disable the analog soft ramp */
  counter += CODEC_IO_Write(dev, DeviceAddr, CS43L22_REG_ANALOG_ZC_SR_SETT, 0x00);
  /* Disable the digital soft ramp */
  counter += CODEC_IO_Write(dev, DeviceAddr, CS43L22_REG_MISC_CTL, 0x04);
  /* Disable the limiter attack level */
  counter += CODEC_IO_Write(dev, DeviceAddr, CS43L22_REG_LIMIT_CTL1, 0x00);
  /* Adjust Bass and Treble levels */
  counter += CODEC_IO_Write(dev, DeviceAddr, CS43L22_REG_TONE_CTL, 0x0F);
  /* Adjust PCM volume level */
  counter += CODEC_IO_Write(dev, DeviceAddr, CS43L22_REG_PCMA_VOL, 0x0A);
  counter += CODEC_IO_Write(dev, DeviceAddr, CS43L22_REG_PCMB_VOL, 0x0A);
  
  /* Return communication control value */
  return counter;  
}

/**
  * @brief  Deinitializes the audio codec.
  * @param  None
  * @retval  None
  */
void cs43l22_DeInit(void)
{
}

/**
  * @brief  Get the CS43L22 ID.
  * @param DeviceAddr: Device address on communication Bus.   
  * @retval The CS43L22 ID 
  */
uint32_t cs43l22_ReadID(struct rt_i2c_bus_device *dev, uint16_t DeviceAddr)
{
  uint8_t Value = 0;
  
  Value = CODEC_IO_Read(dev, DeviceAddr, CS43L22_CHIPID_ADDR);
  Value = (Value & CS43L22_ID_MASK);
  
  return((uint32_t) Value);
}

/**
  * @brief Start the audio Codec play feature.
  * @note For this codec no Play options are required.
  * @param DeviceAddr: Device address on communication Bus.   
  * @retval 0 if correct communication, else wrong communication
  */
uint32_t cs43l22_Play(struct rt_i2c_bus_device *dev, uint16_t DeviceAddr)
{
  uint32_t counter = 0;
  
  if(Is_cs43l22_Stop == 1)
  {
	LOG_D("%s %d", __func__, __LINE__);
    /* Enable the digital soft ramp */
    counter += CODEC_IO_Write(dev, DeviceAddr, CS43L22_REG_MISC_CTL, 0x06);
  
    /* Enable Output device */  
    counter += cs43l22_SetMute(dev, DeviceAddr, AUDIO_MUTE_OFF);
    
    /* Power on the Codec */
    counter += CODEC_IO_Write(dev, DeviceAddr, CS43L22_REG_POWER_CTL1, 0x9E);  
    Is_cs43l22_Stop = 0;
  }
  
  /* Return communication control value */
  return counter;  
}

/**
  * @brief Pauses playing on the audio codec.
  * @param DeviceAddr: Device address on communication Bus. 
  * @retval 0 if correct communication, else wrong communication
  */
uint32_t cs43l22_Pause(struct rt_i2c_bus_device *dev, uint16_t DeviceAddr)
{  
  uint32_t counter = 0;
 
  /* Pause the audio file playing */
  /* Mute the output first */
  counter += cs43l22_SetMute(dev, DeviceAddr, AUDIO_MUTE_ON);
  
  /* Put the Codec in Power save mode */    
  counter += CODEC_IO_Write(dev, DeviceAddr, CS43L22_REG_POWER_CTL1, 0x01);
 
  return counter;
}

/**
  * @brief Resumes playing on the audio codec.
  * @param DeviceAddr: Device address on communication Bus. 
  * @retval 0 if correct communication, else wrong communication
  */
uint32_t cs43l22_Resume(struct rt_i2c_bus_device *dev,
		uint16_t DeviceAddr)
{
  uint32_t counter = 0;
  volatile uint32_t index = 0x00;
  /* Resumes the audio file playing */  
  /* Unmute the output first */
  counter += cs43l22_SetMute(dev, DeviceAddr, AUDIO_MUTE_OFF);

  for(index = 0x00; index < 0xFF; index++);
  
  counter += CODEC_IO_Write(dev, DeviceAddr, CS43L22_REG_POWER_CTL2, OutputDev);

  /* Exit the Power save mode */
  counter += CODEC_IO_Write(dev, DeviceAddr, CS43L22_REG_POWER_CTL1, 0x9E); 
  
  return counter;
}

/**
  * @brief Stops audio Codec playing. It powers down the codec.
  * @param DeviceAddr: Device address on communication Bus. 
  * @param CodecPdwnMode: selects the  power down mode.
  *          - CODEC_PDWN_HW: Physically power down the codec. When resuming from this
  *                           mode, the codec is set to default configuration 
  *                           (user should re-Initialize the codec in order to 
  *                            play again the audio stream).
  * @retval 0 if correct communication, else wrong communication
  */
uint32_t cs43l22_Stop(struct rt_i2c_bus_device *dev,
		uint16_t DeviceAddr)
{
  uint32_t counter = 0;
  
	LOG_D("%s %d", __func__, __LINE__);
  /* Mute the output first */
  counter += cs43l22_SetMute(dev, DeviceAddr, AUDIO_MUTE_ON);

  /* Disable the digital soft ramp */
  counter += CODEC_IO_Write(dev, DeviceAddr, CS43L22_REG_MISC_CTL, 0x04);
  
  /* Power down the DAC and the speaker (PMDAC and PMSPK bits)*/
  counter += CODEC_IO_Write(dev, DeviceAddr, CS43L22_REG_POWER_CTL1, 0x9F);
  
  Is_cs43l22_Stop = 1;
  return counter;    
}

/**
  * @brief Sets higher or lower the codec volume level.
  * @param DeviceAddr: Device address on communication Bus.   
  * @param Volume: a byte value from 0 to 255 (refer to codec registers 
  *                description for more details).
  *         
  * @retval 0 if correct communication, else wrong communication
  */
uint32_t cs43l22_SetVolume(struct rt_i2c_bus_device *dev,
		uint16_t DeviceAddr, uint8_t Volume)
{
  uint32_t counter = 0;
  uint8_t convertedvol = VOLUME_CONVERT(Volume);

	LOG_D("%s %d", __func__, __LINE__);
  if(convertedvol > 0xE6)
  {
    /* Set the Master volume */
    counter += CODEC_IO_Write(dev, DeviceAddr, CS43L22_REG_MASTER_A_VOL, convertedvol - 0xE7); 
    counter += CODEC_IO_Write(dev, DeviceAddr, CS43L22_REG_MASTER_B_VOL, convertedvol - 0xE7);     
  }
  else
  {
    /* Set the Master volume */
    counter += CODEC_IO_Write(dev, DeviceAddr, CS43L22_REG_MASTER_A_VOL, convertedvol + 0x19); 
    counter += CODEC_IO_Write(dev, DeviceAddr, CS43L22_REG_MASTER_B_VOL, convertedvol + 0x19); 
  }

  return counter;
}

/**
  * @brief Sets new frequency.
  * @param DeviceAddr: Device address on communication Bus.   
  * @param AudioFreq: Audio frequency used to play the audio stream.
  * @retval 0 if correct communication, else wrong communication
  */
uint32_t cs43l22_SetFrequency(struct rt_i2c_bus_device *dev, uint16_t DeviceAddr,
		uint32_t AudioFreq)
{
  return 0;
}

/**
  * @brief Enables or disables the mute feature on the audio codec.
  * @param DeviceAddr: Device address on communication Bus.   
  * @param Cmd: AUDIO_MUTE_ON to enable the mute or AUDIO_MUTE_OFF to disable the
  *             mute mode.
  * @retval 0 if correct communication, else wrong communication
  */
uint32_t cs43l22_SetMute(struct rt_i2c_bus_device *dev,
		uint16_t DeviceAddr, uint32_t Cmd)
{
  uint32_t counter = 0;
  
	LOG_D("%s %d, mute %d", __func__, __LINE__, Cmd);
  /* Set the Mute mode */
  if(Cmd == AUDIO_MUTE_ON)
  {
    counter += CODEC_IO_Write(dev, DeviceAddr, CS43L22_REG_POWER_CTL2, 0xFF);
    counter += CODEC_IO_Write(dev, DeviceAddr, CS43L22_REG_HEADPHONE_A_VOL, 0x01);
    counter += CODEC_IO_Write(dev, DeviceAddr, CS43L22_REG_HEADPHONE_B_VOL, 0x01);
  }
  else /* AUDIO_MUTE_OFF Disable the Mute */
  {
    counter += CODEC_IO_Write(dev, DeviceAddr, CS43L22_REG_HEADPHONE_A_VOL, 0x00);
    counter += CODEC_IO_Write(dev, DeviceAddr, CS43L22_REG_HEADPHONE_B_VOL, 0x00);
    counter += CODEC_IO_Write(dev, DeviceAddr, CS43L22_REG_POWER_CTL2, OutputDev);
  }
  return counter;
}

/**
  * @brief Switch dynamically (while audio file is played) the output target 
  *         (speaker or headphone).
  * @note This function modifies a global variable of the audio codec driver: OutputDev.
  * @param DeviceAddr: Device address on communication Bus.
  * @param Output: specifies the audio output target: OUTPUT_DEVICE_SPEAKER,
  *         OUTPUT_DEVICE_HEADPHONE, OUTPUT_DEVICE_BOTH or OUTPUT_DEVICE_AUTO 
  * @retval 0 if correct communication, else wrong communication
  */
uint32_t cs43l22_SetOutputMode(struct rt_i2c_bus_device *dev,
		uint16_t DeviceAddr, uint8_t Output)
{
  uint32_t counter = 0; 
  
	LOG_D("%s %d", __func__, __LINE__);
  switch (Output) 
  {
    case OUTPUT_DEVICE_SPEAKER:
      counter += CODEC_IO_Write(dev, DeviceAddr, CS43L22_REG_POWER_CTL2, 0xFA); /* SPK always ON & HP always OFF */
      OutputDev = 0xFA;
      break;
      
    case OUTPUT_DEVICE_HEADPHONE:
      counter += CODEC_IO_Write(dev, DeviceAddr, CS43L22_REG_POWER_CTL2, 0xAF); /* SPK always OFF & HP always ON */
      OutputDev = 0xAF;
      break;
      
    case OUTPUT_DEVICE_BOTH:
      counter += CODEC_IO_Write(dev, DeviceAddr, CS43L22_REG_POWER_CTL2, 0xAA); /* SPK always ON & HP always ON */
      OutputDev = 0xAA;
      break;
      
    case OUTPUT_DEVICE_AUTO:
      counter += CODEC_IO_Write(dev, DeviceAddr, CS43L22_REG_POWER_CTL2, 0x05); /* Detect the HP or the SPK automatically */
      OutputDev = 0x05;
      break;    
      
    default:
      counter += CODEC_IO_Write(dev, DeviceAddr, CS43L22_REG_POWER_CTL2, 0x05); /* Detect the HP or the SPK automatically */
      OutputDev = 0x05;
      break;
  }  
  return counter;
}

/**
  * @brief Resets cs43l22 registers.
  * @param DeviceAddr: Device address on communication Bus. 
  * @retval 0 if correct communication, else wrong communication
  */
uint32_t cs43l22_Reset(uint16_t DeviceAddr)
{
  return 0;
}

static uint8_t CODEC_IO_Write(struct rt_i2c_bus_device *dev, uint8_t Addr,
		uint8_t Reg, uint8_t Value)
{
  struct rt_i2c_msg msg;
  rt_uint8_t send_buffer[2];
  RT_ASSERT(dev != RT_NULL);
  
  send_buffer[0] = Reg;
  send_buffer[1] = Value;
  msg.addr = Addr;
  msg.flags = RT_I2C_WR;
  msg.len = 2;
  msg.buf = send_buffer;
  rt_i2c_transfer(dev, &msg, 1);

  return 1;
}

static uint8_t CODEC_IO_Read(struct rt_i2c_bus_device *dev, uint8_t Addr,
		uint8_t Reg)
{
    struct rt_i2c_msg msg[2] = {0, 0};
    uint8_t res = 0;

    RT_ASSERT(dev != RT_NULL);

    msg[0].addr  = Addr;    /* Slave address */
    msg[0].flags = RT_I2C_WR;        /* Write flag */
    msg[0].buf   = &Reg;             /* Slave register address */
    msg[0].len   = 1;                /* Number of bytes sent */

    msg[1].addr  = Addr;
    msg[1].flags = RT_I2C_RD;
    msg[1].len   = 1;
    msg[1].buf   = &res;

    if (rt_i2c_transfer(dev, msg, 2) == 2)
    {
    	    LOG_D("codec id %02x", res);
        return res;
    }
	LOG_D("Can't read codec id, %02x", res);
    return 0;
}
