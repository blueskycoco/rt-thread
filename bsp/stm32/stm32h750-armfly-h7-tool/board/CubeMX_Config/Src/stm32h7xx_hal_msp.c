/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * File Name          : stm32h7xx_hal_msp.c
 * Description        : This file provides code for the MSP Initialization 
 *                      and de-Initialization codes.
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN Define */

/* USER CODE END Define */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN Macro */

/* USER CODE END Macro */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* External functions --------------------------------------------------------*/
/* USER CODE BEGIN ExternalFunctions */

/* USER CODE END ExternalFunctions */

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);
/**
 * Initializes the Global MSP.
 */
void HAL_MspInit(void)
{
	/* USER CODE BEGIN MspInit 0 */

	/* USER CODE END MspInit 0 */

	__HAL_RCC_SYSCFG_CLK_ENABLE();
	/* System interrupt init*/

	/* USER CODE BEGIN MspInit 1 */

	/* USER CODE END MspInit 1 */
}

/**
 * @brief ADC MSP Initialization
 * This function configures the hardware resources used in this example
 * @param hadc: ADC handle pointer
 * @retval None
 */
void HAL_ADC_MspInit(ADC_HandleTypeDef* hadc)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	if(hadc->Instance==ADC3)
	{
		/* USER CODE BEGIN ADC3_MspInit 0 */

		/* USER CODE END ADC3_MspInit 0 */
		/* Peripheral clock enable */
		__HAL_RCC_ADC3_CLK_ENABLE();

		__HAL_RCC_GPIOF_CLK_ENABLE();
		/**ADC3 GPIO Configuration    
		  PF3     ------> ADC3_INP5
		  PF8     ------> ADC3_INP7 
		  */
		GPIO_InitStruct.Pin = GPIO_PIN_3|GPIO_PIN_8;
		GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

		/* USER CODE BEGIN ADC3_MspInit 1 */

		/* USER CODE END ADC3_MspInit 1 */
	}

}

/**
 * @brief ADC MSP De-Initialization
 * This function freeze the hardware resources used in this example
 * @param hadc: ADC handle pointer
 * @retval None
 */
void HAL_ADC_MspDeInit(ADC_HandleTypeDef* hadc)
{
	if(hadc->Instance==ADC3)
	{
		/* USER CODE BEGIN ADC3_MspDeInit 0 */

		/* USER CODE END ADC3_MspDeInit 0 */
		/* Peripheral clock disable */
		__HAL_RCC_ADC3_CLK_DISABLE();

		/**ADC3 GPIO Configuration    
		  PF3     ------> ADC3_INP5
		  PF8     ------> ADC3_INP7 
		  */
		HAL_GPIO_DeInit(GPIOF, GPIO_PIN_3|GPIO_PIN_8);

		/* USER CODE BEGIN ADC3_MspDeInit 1 */

		/* USER CODE END ADC3_MspDeInit 1 */
	}

}

/**
 * @brief CRC MSP Initialization
 * This function configures the hardware resources used in this example
 * @param hcrc: CRC handle pointer
 * @retval None
 */
void HAL_CRC_MspInit(CRC_HandleTypeDef* hcrc)
{
	if(hcrc->Instance==CRC)
	{
		/* USER CODE BEGIN CRC_MspInit 0 */

		/* USER CODE END CRC_MspInit 0 */
		/* Peripheral clock enable */
		__HAL_RCC_CRC_CLK_ENABLE();
		/* USER CODE BEGIN CRC_MspInit 1 */

		/* USER CODE END CRC_MspInit 1 */
	}

}

/**
 * @brief CRC MSP De-Initialization
 * This function freeze the hardware resources used in this example
 * @param hcrc: CRC handle pointer
 * @retval None
 */
void HAL_CRC_MspDeInit(CRC_HandleTypeDef* hcrc)
{
	if(hcrc->Instance==CRC)
	{
		/* USER CODE BEGIN CRC_MspDeInit 0 */

		/* USER CODE END CRC_MspDeInit 0 */
		/* Peripheral clock disable */
		__HAL_RCC_CRC_CLK_DISABLE();
		/* USER CODE BEGIN CRC_MspDeInit 1 */

		/* USER CODE END CRC_MspDeInit 1 */
	}

}

/**
 * @brief DAC MSP Initialization
 * This function configures the hardware resources used in this example
 * @param hdac: DAC handle pointer
 * @retval None
 */
void HAL_DAC_MspInit(DAC_HandleTypeDef* hdac)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	if(hdac->Instance==DAC1)
	{
		/* USER CODE BEGIN DAC1_MspInit 0 */

		/* USER CODE END DAC1_MspInit 0 */
		/* Peripheral clock enable */
		__HAL_RCC_DAC12_CLK_ENABLE();

		__HAL_RCC_GPIOA_CLK_ENABLE();
		/**DAC1 GPIO Configuration    
		  PA4     ------> DAC1_OUT1 
		  */
		GPIO_InitStruct.Pin = GPIO_PIN_4;
		GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

		/* USER CODE BEGIN DAC1_MspInit 1 */

		/* USER CODE END DAC1_MspInit 1 */
	}

}

/**
 * @brief DAC MSP De-Initialization
 * This function freeze the hardware resources used in this example
 * @param hdac: DAC handle pointer
 * @retval None
 */
void HAL_DAC_MspDeInit(DAC_HandleTypeDef* hdac)
{
	if(hdac->Instance==DAC1)
	{
		/* USER CODE BEGIN DAC1_MspDeInit 0 */

		/* USER CODE END DAC1_MspDeInit 0 */
		/* Peripheral clock disable */
		__HAL_RCC_DAC12_CLK_DISABLE();

		/**DAC1 GPIO Configuration    
		  PA4     ------> DAC1_OUT1 
		  */
		HAL_GPIO_DeInit(GPIOA, GPIO_PIN_4);

		/* USER CODE BEGIN DAC1_MspDeInit 1 */

		/* USER CODE END DAC1_MspDeInit 1 */
	}

}

/**
 * @brief ETH MSP Initialization
 * This function configures the hardware resources used in this example
 * @param heth: ETH handle pointer
 * @retval None
 */
void HAL_ETH_MspInit(ETH_HandleTypeDef* heth)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	if(heth->Instance==ETH)
	{
		/* USER CODE BEGIN ETH_MspInit 0 */

		/* USER CODE END ETH_MspInit 0 */
		/* Peripheral clock enable */
		__HAL_RCC_ETH1MAC_CLK_ENABLE();
		__HAL_RCC_ETH1TX_CLK_ENABLE();
		__HAL_RCC_ETH1RX_CLK_ENABLE();

		__HAL_RCC_GPIOG_CLK_ENABLE();
		__HAL_RCC_GPIOC_CLK_ENABLE();
		__HAL_RCC_GPIOA_CLK_ENABLE();
		/**ETH GPIO Configuration    
		  PG14     ------> ETH_TXD1
		  PG13     ------> ETH_TXD0
		  PG11     ------> ETH_TX_EN
		  PC1     ------> ETH_MDC
		  PA1     ------> ETH_REF_CLK
		  PC4     ------> ETH_RXD0
		  PA2     ------> ETH_MDIO
		  PC5     ------> ETH_RXD1
		  PA7     ------> ETH_CRS_DV 
		  */
		GPIO_InitStruct.Pin = GPIO_PIN_14|GPIO_PIN_13|GPIO_PIN_11;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
		GPIO_InitStruct.Alternate = GPIO_AF11_ETH;
		HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

		GPIO_InitStruct.Pin = GPIO_PIN_1|GPIO_PIN_4|GPIO_PIN_5;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
		GPIO_InitStruct.Alternate = GPIO_AF11_ETH;
		HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

		GPIO_InitStruct.Pin = GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_7;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
		GPIO_InitStruct.Alternate = GPIO_AF11_ETH;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

		/* USER CODE BEGIN ETH_MspInit 1 */

		/* USER CODE END ETH_MspInit 1 */
	}

}

/**
 * @brief ETH MSP De-Initialization
 * This function freeze the hardware resources used in this example
 * @param heth: ETH handle pointer
 * @retval None
 */
void HAL_ETH_MspDeInit(ETH_HandleTypeDef* heth)
{
	if(heth->Instance==ETH)
	{
		/* USER CODE BEGIN ETH_MspDeInit 0 */

		/* USER CODE END ETH_MspDeInit 0 */
		/* Peripheral clock disable */
		__HAL_RCC_ETH1MAC_CLK_DISABLE();
		__HAL_RCC_ETH1TX_CLK_DISABLE();
		__HAL_RCC_ETH1RX_CLK_DISABLE();

		/**ETH GPIO Configuration    
		  PG14     ------> ETH_TXD1
		  PG13     ------> ETH_TXD0
		  PG11     ------> ETH_TX_EN
		  PC1     ------> ETH_MDC
		  PA1     ------> ETH_REF_CLK
		  PC4     ------> ETH_RXD0
		  PA2     ------> ETH_MDIO
		  PC5     ------> ETH_RXD1
		  PA7     ------> ETH_CRS_DV 
		  */
		HAL_GPIO_DeInit(GPIOG, GPIO_PIN_14|GPIO_PIN_13|GPIO_PIN_11);

		HAL_GPIO_DeInit(GPIOC, GPIO_PIN_1|GPIO_PIN_4|GPIO_PIN_5);

		HAL_GPIO_DeInit(GPIOA, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_7);

		/* USER CODE BEGIN ETH_MspDeInit 1 */

		/* USER CODE END ETH_MspDeInit 1 */
	}

}

/**
 * @brief QSPI MSP Initialization
 * This function configures the hardware resources used in this example
 * @param hqspi: QSPI handle pointer
 * @retval None
 */
void HAL_QSPI_MspInit(QSPI_HandleTypeDef* hqspi)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	if(hqspi->Instance==QUADSPI)
	{
		/* USER CODE BEGIN QUADSPI_MspInit 0 */

		/* USER CODE END QUADSPI_MspInit 0 */
		/* Peripheral clock enable */
		__HAL_RCC_QSPI_CLK_ENABLE();

		__HAL_RCC_GPIOG_CLK_ENABLE();
		__HAL_RCC_GPIOF_CLK_ENABLE();
		/**QUADSPI GPIO Configuration    
		  PG6     ------> QUADSPI_BK1_NCS
		  PF7     ------> QUADSPI_BK1_IO2
		  PF10     ------> QUADSPI_CLK
		  PF6     ------> QUADSPI_BK1_IO3
		  PF9     ------> QUADSPI_BK1_IO1
		  PF8     ------> QUADSPI_BK1_IO0 
		  */
		GPIO_InitStruct.Pin = GPIO_PIN_6;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
		GPIO_InitStruct.Alternate = GPIO_AF10_QUADSPI;
		HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

		GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_10;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
		GPIO_InitStruct.Alternate = GPIO_AF9_QUADSPI;
		HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);
		GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9;
		GPIO_InitStruct.Alternate = GPIO_AF10_QUADSPI;
		HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

		/* USER CODE BEGIN QUADSPI_MspInit 1 */

		/* USER CODE END QUADSPI_MspInit 1 */
	}

}

/**
 * @brief QSPI MSP De-Initialization
 * This function freeze the hardware resources used in this example
 * @param hqspi: QSPI handle pointer
 * @retval None
 */
void HAL_QSPI_MspDeInit(QSPI_HandleTypeDef* hqspi)
{
	if(hqspi->Instance==QUADSPI)
	{
		/* USER CODE BEGIN QUADSPI_MspDeInit 0 */

		/* USER CODE END QUADSPI_MspDeInit 0 */
		/* Peripheral clock disable */
		__HAL_RCC_QSPI_CLK_DISABLE();

		/**QUADSPI GPIO Configuration    
		  PG6     ------> QUADSPI_BK1_NCS
		  PF7     ------> QUADSPI_BK1_IO2
		  PB2     ------> QUADSPI_CLK
		  PD13     ------> QUADSPI_BK1_IO3
		  PD12     ------> QUADSPI_BK1_IO1
		  PD11     ------> QUADSPI_BK1_IO0 
		  */
		HAL_GPIO_DeInit(GPIOG, GPIO_PIN_6);

		HAL_GPIO_DeInit(GPIOF, GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10);

		/* USER CODE BEGIN QUADSPI_MspDeInit 1 */

		/* USER CODE END QUADSPI_MspDeInit 1 */
	}

}

/**
 * @brief RNG MSP Initialization
 * This function configures the hardware resources used in this example
 * @param hrng: RNG handle pointer
 * @retval None
 */
void HAL_RNG_MspInit(RNG_HandleTypeDef* hrng)
{
	if(hrng->Instance==RNG)
	{
		/* USER CODE BEGIN RNG_MspInit 0 */

		/* USER CODE END RNG_MspInit 0 */
		/* Peripheral clock enable */
		__HAL_RCC_RNG_CLK_ENABLE();
		/* USER CODE BEGIN RNG_MspInit 1 */

		/* USER CODE END RNG_MspInit 1 */
	}

}

/**
 * @brief RNG MSP De-Initialization
 * This function freeze the hardware resources used in this example
 * @param hrng: RNG handle pointer
 * @retval None
 */
void HAL_RNG_MspDeInit(RNG_HandleTypeDef* hrng)
{
	if(hrng->Instance==RNG)
	{
		/* USER CODE BEGIN RNG_MspDeInit 0 */

		/* USER CODE END RNG_MspDeInit 0 */
		/* Peripheral clock disable */
		__HAL_RCC_RNG_CLK_DISABLE();
		/* USER CODE BEGIN RNG_MspDeInit 1 */

		/* USER CODE END RNG_MspDeInit 1 */
	}

}

/**
 * @brief MMC MSP Initialization
 * This function configures the hardware resources used in this example
 * @param hmmc: MMC handle pointer
 * @retval None
 */
void HAL_MMC_MspInit(MMC_HandleTypeDef* hmmc)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	if(hmmc->Instance==SDMMC1)
	{
		/* USER CODE BEGIN SDMMC1_MspInit 0 */

		/* USER CODE END SDMMC1_MspInit 0 */
		/* Peripheral clock enable */
		__HAL_RCC_SDMMC1_CLK_ENABLE();

		__HAL_RCC_GPIOB_CLK_ENABLE();
		__HAL_RCC_GPIOC_CLK_ENABLE();
		__HAL_RCC_GPIOD_CLK_ENABLE();
		/**SDMMC1 GPIO Configuration    
		  PB8     ------> SDMMC1_D4
		  PC12     ------> SDMMC1_CK
		  PB9     ------> SDMMC1_D5
		  PC11     ------> SDMMC1_D3
		  PC10     ------> SDMMC1_D2
		  PD2     ------> SDMMC1_CMD
		  PC9     ------> SDMMC1_D1
		  PC8     ------> SDMMC1_D0
		  PC7     ------> SDMMC1_D7
		  PC6     ------> SDMMC1_D6 
		  */
		GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
		GPIO_InitStruct.Alternate = GPIO_AF12_SDIO1;
		HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

		GPIO_InitStruct.Pin = GPIO_PIN_12|GPIO_PIN_11|GPIO_PIN_10|GPIO_PIN_9 
			|GPIO_PIN_8|GPIO_PIN_7|GPIO_PIN_6;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
		GPIO_InitStruct.Alternate = GPIO_AF12_SDIO1;
		HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

		GPIO_InitStruct.Pin = GPIO_PIN_2;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
		GPIO_InitStruct.Alternate = GPIO_AF12_SDIO1;
		HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

		/* USER CODE BEGIN SDMMC1_MspInit 1 */

		/* USER CODE END SDMMC1_MspInit 1 */
	}

}

/**
 * @brief MMC MSP De-Initialization
 * This function freeze the hardware resources used in this example
 * @param hmmc: MMC handle pointer
 * @retval None
 */
void HAL_MMC_MspDeInit(MMC_HandleTypeDef* hmmc)
{
	if(hmmc->Instance==SDMMC1)
	{
		/* USER CODE BEGIN SDMMC1_MspDeInit 0 */

		/* USER CODE END SDMMC1_MspDeInit 0 */
		/* Peripheral clock disable */
		__HAL_RCC_SDMMC1_CLK_DISABLE();

		/**SDMMC1 GPIO Configuration    
		  PB8     ------> SDMMC1_D4
		  PC12     ------> SDMMC1_CK
		  PB9     ------> SDMMC1_D5
		  PC11     ------> SDMMC1_D3
		  PC10     ------> SDMMC1_D2
		  PD2     ------> SDMMC1_CMD
		  PC9     ------> SDMMC1_D1
		  PC8     ------> SDMMC1_D0
		  PC7     ------> SDMMC1_D7
		  PC6     ------> SDMMC1_D6 
		  */
		HAL_GPIO_DeInit(GPIOB, GPIO_PIN_8|GPIO_PIN_9);

		HAL_GPIO_DeInit(GPIOC, GPIO_PIN_12|GPIO_PIN_11|GPIO_PIN_10|GPIO_PIN_9 
				|GPIO_PIN_8|GPIO_PIN_7|GPIO_PIN_6);

		HAL_GPIO_DeInit(GPIOD, GPIO_PIN_2);

		/* USER CODE BEGIN SDMMC1_MspDeInit 1 */

		/* USER CODE END SDMMC1_MspDeInit 1 */
	}

}

/**
 * @brief TIM_Base MSP Initialization
 * This function configures the hardware resources used in this example
 * @param htim_base: TIM_Base handle pointer
 * @retval None
 */
void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* htim_base)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	if(htim_base->Instance==TIM12)
	{
		/* USER CODE BEGIN TIM12_MspInit 0 */

		/* USER CODE END TIM12_MspInit 0 */
		/* Peripheral clock enable */
		__HAL_RCC_TIM12_CLK_ENABLE();

		__HAL_RCC_GPIOB_CLK_ENABLE();
		/**TIM12 GPIO Configuration    
		  PB14     ------> TIM12_CH1 
		  */
		GPIO_InitStruct.Pin = GPIO_PIN_14;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
		GPIO_InitStruct.Alternate = GPIO_AF2_TIM12;
		HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

		/* USER CODE BEGIN TIM12_MspInit 1 */

		/* USER CODE END TIM12_MspInit 1 */
	}

}

void HAL_TIM_MspPostInit(TIM_HandleTypeDef* htim)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	if(htim->Instance==TIM12)
	{
		/* USER CODE BEGIN TIM12_MspPostInit 0 */

		/* USER CODE END TIM12_MspPostInit 0 */

		__HAL_RCC_GPIOH_CLK_ENABLE();
		/**TIM12 GPIO Configuration    
		  PH9     ------> TIM12_CH2 
		  */
		GPIO_InitStruct.Pin = GPIO_PIN_9;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
		GPIO_InitStruct.Alternate = GPIO_AF2_TIM12;
		HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);

		/* USER CODE BEGIN TIM12_MspPostInit 1 */

		/* USER CODE END TIM12_MspPostInit 1 */
	}

}
/**
 * @brief TIM_Base MSP De-Initialization
 * This function freeze the hardware resources used in this example
 * @param htim_base: TIM_Base handle pointer
 * @retval None
 */
void HAL_TIM_Base_MspDeInit(TIM_HandleTypeDef* htim_base)
{
	if(htim_base->Instance==TIM12)
	{
		/* USER CODE BEGIN TIM12_MspDeInit 0 */

		/* USER CODE END TIM12_MspDeInit 0 */
		/* Peripheral clock disable */
		__HAL_RCC_TIM12_CLK_DISABLE();

		/**TIM12 GPIO Configuration    
		  PH9     ------> TIM12_CH2
		  PB14     ------> TIM12_CH1 
		  */
		HAL_GPIO_DeInit(GPIOH, GPIO_PIN_9);

		HAL_GPIO_DeInit(GPIOB, GPIO_PIN_14);

		/* USER CODE BEGIN TIM12_MspDeInit 1 */

		/* USER CODE END TIM12_MspDeInit 1 */
	}

}

/**
 * @brief UART MSP Initialization
 * This function configures the hardware resources used in this example
 * @param huart: UART handle pointer
 * @retval None
 */
void HAL_UART_MspInit(UART_HandleTypeDef* huart)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	if(huart->Instance==UART4)
	{
		/* USER CODE BEGIN UART4_MspInit 0 */

		/* USER CODE END UART4_MspInit 0 */
		/* Peripheral clock enable */
		__HAL_RCC_UART4_CLK_ENABLE();

		__HAL_RCC_GPIOA_CLK_ENABLE();
		__HAL_RCC_GPIOI_CLK_ENABLE();
		/**UART4 GPIO Configuration    
		  PA0     ------> UART4_TX
		  PI8     ------> UART4_RX 
		  */
		GPIO_InitStruct.Pin = GPIO_PIN_9;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
		GPIO_InitStruct.Alternate = GPIO_AF8_UART4;
		HAL_GPIO_Init(GPIOI, &GPIO_InitStruct);
		GPIO_InitStruct.Pin = GPIO_PIN_0;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

		/* USER CODE BEGIN UART4_MspInit 1 */

		/* USER CODE END UART4_MspInit 1 */
	}
	else if(huart->Instance==USART1)
	{
		/* USER CODE BEGIN USART1_MspInit 0 */

		/* USER CODE END USART1_MspInit 0 */
		/* Peripheral clock enable */
		__HAL_RCC_USART1_CLK_ENABLE();

		__HAL_RCC_GPIOA_CLK_ENABLE();
		/**USART1 GPIO Configuration    
		  PA10     ------> USART1_RX
		  PA9     ------> USART1_TX 
		  */
		GPIO_InitStruct.Pin = GPIO_PIN_10|GPIO_PIN_9;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
		GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

		/* USER CODE BEGIN USART1_MspInit 1 */

		/* USER CODE END USART1_MspInit 1 */
	}

}

/**
 * @brief UART MSP De-Initialization
 * This function freeze the hardware resources used in this example
 * @param huart: UART handle pointer
 * @retval None
 */
void HAL_UART_MspDeInit(UART_HandleTypeDef* huart)
{
	if(huart->Instance==UART4)
	{
		/* USER CODE BEGIN UART4_MspDeInit 0 */

		/* USER CODE END UART4_MspDeInit 0 */
		/* Peripheral clock disable */
		__HAL_RCC_UART4_CLK_DISABLE();

		/**UART4 GPIO Configuration    
		  PH13     ------> UART4_TX
		  PH14     ------> UART4_RX 
		  */
		HAL_GPIO_DeInit(GPIOA, GPIO_PIN_0);
		HAL_GPIO_DeInit(GPIOI, GPIO_PIN_9);

		/* USER CODE BEGIN UART4_MspDeInit 1 */

		/* USER CODE END UART4_MspDeInit 1 */
	}
	else if(huart->Instance==USART1)
	{
		/* USER CODE BEGIN USART1_MspDeInit 0 */

		/* USER CODE END USART1_MspDeInit 0 */
		/* Peripheral clock disable */
		__HAL_RCC_USART1_CLK_DISABLE();

		/**USART1 GPIO Configuration    
		  PA10     ------> USART1_RX
		  PA9     ------> USART1_TX 
		  */
		HAL_GPIO_DeInit(GPIOA, GPIO_PIN_10|GPIO_PIN_9);

		/* USER CODE BEGIN USART1_MspDeInit 1 */

		/* USER CODE END USART1_MspDeInit 1 */
	}

}

/**
 * @brief PCD MSP Initialization
 * This function configures the hardware resources used in this example
 * @param hpcd: PCD handle pointer
 * @retval None
 */
void HAL_PCD_MspInit(PCD_HandleTypeDef* hpcd)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	if(hpcd->Instance==USB_OTG_HS)
	{
		/* USER CODE BEGIN USB_OTG_HS_MspInit 0 */

		/* USER CODE END USB_OTG_HS_MspInit 0 */

		__HAL_RCC_GPIOB_CLK_ENABLE();
		__HAL_RCC_GPIOI_CLK_ENABLE();
		__HAL_RCC_GPIOH_CLK_ENABLE();
		__HAL_RCC_GPIOC_CLK_ENABLE();
		__HAL_RCC_GPIOA_CLK_ENABLE();
		/**USB_OTG_HS GPIO Configuration    
		  PB5     ------> USB_OTG_HS_ULPI_D7
		  PI11     ------> USB_OTG_HS_ULPI_DIR
		  PH4     ------> USB_OTG_HS_ULPI_NXT
		  PC0     ------> USB_OTG_HS_ULPI_STP
		  PA5     ------> USB_OTG_HS_ULPI_CK
		  PB12     ------> USB_OTG_HS_ULPI_D5
		  PB13     ------> USB_OTG_HS_ULPI_D6
		  PA3     ------> USB_OTG_HS_ULPI_D0
		  PB1     ------> USB_OTG_HS_ULPI_D2
		  PB0     ------> USB_OTG_HS_ULPI_D1
		  PB10     ------> USB_OTG_HS_ULPI_D3
		  PB11     ------> USB_OTG_HS_ULPI_D4 
		  */
		GPIO_InitStruct.Pin = GPIO_PIN_5|GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_1 
			|GPIO_PIN_0|GPIO_PIN_10|GPIO_PIN_11;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
		GPIO_InitStruct.Alternate = GPIO_AF10_OTG2_HS;
		HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

		GPIO_InitStruct.Pin = GPIO_PIN_11;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
		GPIO_InitStruct.Alternate = GPIO_AF10_OTG2_HS;
		HAL_GPIO_Init(GPIOI, &GPIO_InitStruct);

		GPIO_InitStruct.Pin = GPIO_PIN_4;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
		GPIO_InitStruct.Alternate = GPIO_AF10_OTG2_HS;
		HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);

		GPIO_InitStruct.Pin = GPIO_PIN_0;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
		GPIO_InitStruct.Alternate = GPIO_AF10_OTG2_HS;
		HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

		GPIO_InitStruct.Pin = GPIO_PIN_5|GPIO_PIN_3;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
		GPIO_InitStruct.Alternate = GPIO_AF10_OTG2_HS;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

		/* Peripheral clock enable */
		__HAL_RCC_USB_OTG_HS_CLK_ENABLE();
		__HAL_RCC_USB_OTG_HS_ULPI_CLK_ENABLE();
		/* USER CODE BEGIN USB_OTG_HS_MspInit 1 */

		/* USER CODE END USB_OTG_HS_MspInit 1 */
	} else if(hpcd->Instance==USB_OTG_FS) {
		__HAL_RCC_GPIOA_CLK_ENABLE();
		
		GPIO_InitStruct.Pin = GPIO_PIN_11|GPIO_PIN_12;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
		GPIO_InitStruct.Alternate = GPIO_AF10_OTG1_FS;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

		__HAL_RCC_USB_OTG_FS_CLK_ENABLE();
	}

}

/**
 * @brief PCD MSP De-Initialization
 * This function freeze the hardware resources used in this example
 * @param hpcd: PCD handle pointer
 * @retval None
 */
void HAL_PCD_MspDeInit(PCD_HandleTypeDef* hpcd)
{
	if(hpcd->Instance==USB_OTG_HS)
	{
		/* USER CODE BEGIN USB_OTG_HS_MspDeInit 0 */

		/* USER CODE END USB_OTG_HS_MspDeInit 0 */
		/* Peripheral clock disable */
		__HAL_RCC_USB_OTG_HS_CLK_DISABLE();
		__HAL_RCC_USB_OTG_HS_ULPI_CLK_DISABLE();

		/**USB_OTG_HS GPIO Configuration    
		  PB5     ------> USB_OTG_HS_ULPI_D7
		  PI11     ------> USB_OTG_HS_ULPI_DIR
		  PH4     ------> USB_OTG_HS_ULPI_NXT
		  PC0     ------> USB_OTG_HS_ULPI_STP
		  PA5     ------> USB_OTG_HS_ULPI_CK
		  PB12     ------> USB_OTG_HS_ULPI_D5
		  PB13     ------> USB_OTG_HS_ULPI_D6
		  PA3     ------> USB_OTG_HS_ULPI_D0
		  PB1     ------> USB_OTG_HS_ULPI_D2
		  PB0     ------> USB_OTG_HS_ULPI_D1
		  PB10     ------> USB_OTG_HS_ULPI_D3
		  PB11     ------> USB_OTG_HS_ULPI_D4 
		  */
		HAL_GPIO_DeInit(GPIOB, GPIO_PIN_5|GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_1 
				|GPIO_PIN_0|GPIO_PIN_10|GPIO_PIN_11);

		HAL_GPIO_DeInit(GPIOI, GPIO_PIN_11);

		HAL_GPIO_DeInit(GPIOH, GPIO_PIN_4);

		HAL_GPIO_DeInit(GPIOC, GPIO_PIN_0);

		HAL_GPIO_DeInit(GPIOA, GPIO_PIN_5|GPIO_PIN_3);

		/* USER CODE BEGIN USB_OTG_HS_MspDeInit 1 */

		/* USER CODE END USB_OTG_HS_MspDeInit 1 */
	} else if(hpcd->Instance==USB_OTG_FS)
		  {
		  	   __HAL_RCC_USB_OTG_FS_CLK_DISABLE();
		  }

}

static uint32_t FMC_Initialized = 0;

static void HAL_FMC_MspInit(void){
  /* USER CODE BEGIN FMC_MspInit 0 */

  /* USER CODE END FMC_MspInit 0 */
  GPIO_InitTypeDef GPIO_InitStruct ={0};
  if (FMC_Initialized) {
    return;
  }
  FMC_Initialized = 1;

  /* Peripheral clock enable */
  __HAL_RCC_FMC_CLK_ENABLE();
  //HAL_SetFMCMemorySwappingConfig(FMC_SWAPBMAP_SDRAM_SRAM);

  /** FMC GPIO Configuration
  PE1   ------> FMC_NBL1
  PE0   ------> FMC_NBL0
  PG15   ------> FMC_SDNCAS
  PD0   ------> FMC_D2
  PD1   ------> FMC_D3
  PG8   ------> FMC_SDCLK
  PF2   ------> FMC_A2
  PF1   ------> FMC_A1
  PF0   ------> FMC_A0
  PG5   ------> FMC_BA1
  PF3   ------> FMC_A3
  PG4   ------> FMC_BA0
  PG2   ------> FMC_A12
  PF5   ------> FMC_A5
  PF4   ------> FMC_A4
  PC2   ------> FMC_SDNE0
  PC3   ------> FMC_SDCKE0
  PE10   ------> FMC_D7
  PH5   ------> FMC_SDNWE
  PF13   ------> FMC_A7
  PF14   ------> FMC_A8
  PE9   ------> FMC_D6
  PE11   ------> FMC_D8
  PD15   ------> FMC_D1
  PD14   ------> FMC_D0
  PF12   ------> FMC_A6
  PF15   ------> FMC_A9
  PE12   ------> FMC_D9
  PE15   ------> FMC_D12
  PF11   ------> FMC_SDNRAS
  PG0   ------> FMC_A10
  PE8   ------> FMC_D5
  PE13   ------> FMC_D10
  PD10   ------> FMC_D15
  PD9   ------> FMC_D14
  PG1   ------> FMC_A11
  PE7   ------> FMC_D4
  PE14   ------> FMC_D11
  PD8   ------> FMC_D13
  */
  GPIO_InitStruct.Pin = GPIO_PIN_1|GPIO_PIN_0|GPIO_PIN_10|GPIO_PIN_9
                          |GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_15|GPIO_PIN_8
                          |GPIO_PIN_13|GPIO_PIN_7|GPIO_PIN_14;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_15|GPIO_PIN_8|GPIO_PIN_5|GPIO_PIN_4
                          |GPIO_PIN_2|GPIO_PIN_0|GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_15|GPIO_PIN_14
                          |GPIO_PIN_10|GPIO_PIN_9|GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_1|GPIO_PIN_0|GPIO_PIN_3
                          |GPIO_PIN_5|GPIO_PIN_4|GPIO_PIN_13|GPIO_PIN_14
                          |GPIO_PIN_12|GPIO_PIN_15|GPIO_PIN_11;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
  HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);

  /* USER CODE BEGIN FMC_MspInit 1 */

  /* USER CODE END FMC_MspInit 1 */
}

void HAL_SDRAM_MspInit(SDRAM_HandleTypeDef* hsdram){
  /* USER CODE BEGIN SDRAM_MspInit 0 */

  /* USER CODE END SDRAM_MspInit 0 */
  HAL_FMC_MspInit();
  /* USER CODE BEGIN SDRAM_MspInit 1 */

  /* USER CODE END SDRAM_MspInit 1 */
}

static uint32_t FMC_DeInitialized = 0;

static void HAL_FMC_MspDeInit(void){
  /* USER CODE BEGIN FMC_MspDeInit 0 */

  /* USER CODE END FMC_MspDeInit 0 */
  if (FMC_DeInitialized) {
    return;
  }
  FMC_DeInitialized = 1;
  /* Peripheral clock enable */
  __HAL_RCC_FMC_CLK_DISABLE();

  /** FMC GPIO Configuration
  PE1   ------> FMC_NBL1
  PE0   ------> FMC_NBL0
  PG15   ------> FMC_SDNCAS
  PD0   ------> FMC_D2
  PD1   ------> FMC_D3
  PG8   ------> FMC_SDCLK
  PF2   ------> FMC_A2
  PF1   ------> FMC_A1
  PF0   ------> FMC_A0
  PG5   ------> FMC_BA1
  PF3   ------> FMC_A3
  PG4   ------> FMC_BA0
  PG2   ------> FMC_A12
  PF5   ------> FMC_A5
  PF4   ------> FMC_A4
  PC2   ------> FMC_SDNE0
  PC3   ------> FMC_SDCKE0
  PE10   ------> FMC_D7
  PH5   ------> FMC_SDNWE
  PF13   ------> FMC_A7
  PF14   ------> FMC_A8
  PE9   ------> FMC_D6
  PE11   ------> FMC_D8
  PD15   ------> FMC_D1
  PD14   ------> FMC_D0
  PF12   ------> FMC_A6
  PF15   ------> FMC_A9
  PE12   ------> FMC_D9
  PE15   ------> FMC_D12
  PF11   ------> FMC_SDNRAS
  PG0   ------> FMC_A10
  PE8   ------> FMC_D5
  PE13   ------> FMC_D10
  PD10   ------> FMC_D15
  PD9   ------> FMC_D14
  PG1   ------> FMC_A11
  PE7   ------> FMC_D4
  PE14   ------> FMC_D11
  PD8   ------> FMC_D13
  */
  HAL_GPIO_DeInit(GPIOE, GPIO_PIN_1|GPIO_PIN_0|GPIO_PIN_10|GPIO_PIN_9
                          |GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_15|GPIO_PIN_8
                          |GPIO_PIN_13|GPIO_PIN_7|GPIO_PIN_14);

  HAL_GPIO_DeInit(GPIOG, GPIO_PIN_15|GPIO_PIN_8|GPIO_PIN_5|GPIO_PIN_4
                          |GPIO_PIN_2|GPIO_PIN_0|GPIO_PIN_1);

  HAL_GPIO_DeInit(GPIOD, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_15|GPIO_PIN_14
                          |GPIO_PIN_10|GPIO_PIN_9|GPIO_PIN_8);

  HAL_GPIO_DeInit(GPIOF, GPIO_PIN_2|GPIO_PIN_1|GPIO_PIN_0|GPIO_PIN_3
                          |GPIO_PIN_5|GPIO_PIN_4|GPIO_PIN_13|GPIO_PIN_14
                          |GPIO_PIN_12|GPIO_PIN_15|GPIO_PIN_11);

  HAL_GPIO_DeInit(GPIOC, GPIO_PIN_2|GPIO_PIN_3);

  HAL_GPIO_DeInit(GPIOH, GPIO_PIN_5);

  /* USER CODE BEGIN FMC_MspDeInit 1 */

  /* USER CODE END FMC_MspDeInit 1 */
}

void HAL_SDRAM_MspDeInit(SDRAM_HandleTypeDef* hsdram){
  /* USER CODE BEGIN SDRAM_MspDeInit 0 */

  /* USER CODE END SDRAM_MspDeInit 0 */
  HAL_FMC_MspDeInit();
  /* USER CODE BEGIN SDRAM_MspDeInit 1 */

  /* USER CODE END SDRAM_MspDeInit 1 */
}
/* USER CODE BEGIN 1 */
void release_resource()
{
		HAL_GPIO_DeInit(GPIOF, GPIO_PIN_3|GPIO_PIN_8);
		__HAL_RCC_ADC3_CLK_DISABLE();
		__HAL_RCC_CRC_CLK_DISABLE();
		__HAL_RCC_DAC12_CLK_DISABLE();
		HAL_GPIO_DeInit(GPIOA, GPIO_PIN_4);
		__HAL_RCC_ETH1MAC_CLK_DISABLE();
		__HAL_RCC_ETH1TX_CLK_DISABLE();
		__HAL_RCC_ETH1RX_CLK_DISABLE();
		HAL_GPIO_DeInit(GPIOG, GPIO_PIN_14|GPIO_PIN_13|GPIO_PIN_11);
		HAL_GPIO_DeInit(GPIOC, GPIO_PIN_1|GPIO_PIN_4|GPIO_PIN_5);
		HAL_GPIO_DeInit(GPIOA, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_7);
		__HAL_RCC_QSPI_CLK_DISABLE();
		HAL_GPIO_DeInit(GPIOG, GPIO_PIN_6);
		HAL_GPIO_DeInit(GPIOF, GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10);
		__HAL_RCC_RNG_CLK_DISABLE();
		__HAL_RCC_SDMMC1_CLK_DISABLE();
		HAL_GPIO_DeInit(GPIOB, GPIO_PIN_8|GPIO_PIN_9);
		HAL_GPIO_DeInit(GPIOC, GPIO_PIN_12|GPIO_PIN_11|GPIO_PIN_10|GPIO_PIN_9 
				|GPIO_PIN_8|GPIO_PIN_7|GPIO_PIN_6);
		HAL_GPIO_DeInit(GPIOD, GPIO_PIN_2);
		__HAL_RCC_TIM12_CLK_DISABLE();
		HAL_GPIO_DeInit(GPIOH, GPIO_PIN_9);
		HAL_GPIO_DeInit(GPIOB, GPIO_PIN_14);
		//__HAL_RCC_UART4_CLK_DISABLE();
		//HAL_GPIO_DeInit(GPIOA, GPIO_PIN_0);
		//HAL_GPIO_DeInit(GPIOI, GPIO_PIN_9);
		__HAL_RCC_USART1_CLK_DISABLE();
		HAL_GPIO_DeInit(GPIOA, GPIO_PIN_10|GPIO_PIN_9);
		__HAL_RCC_USB_OTG_FS_CLK_DISABLE();
		HAL_GPIO_DeInit(GPIOA, GPIO_PIN_11|GPIO_PIN_12);
  		//HAL_FMC_MspDeInit();
}
/* USER CODE END 1 */

/**
* @brief SPI MSP Initialization
* This function configures the hardware resources used in this example
* @param hspi: SPI handle pointer
* @retval None
*/
void HAL_SPI_MspInit(SPI_HandleTypeDef* hspi)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(hspi->Instance==SPI1)
  {
  /* USER CODE BEGIN SPI1_MspInit 0 */

  /* USER CODE END SPI1_MspInit 0 */
    /* Peripheral clock enable */
    __HAL_RCC_SPI1_CLK_ENABLE();

    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOG_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**SPI1 GPIO Configuration
    PB5     ------> SPI1_MOSI
    PG9     ------> SPI1_MISO
    PA5     ------> SPI1_SCK
    */
    GPIO_InitStruct.Pin = GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
    HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* USER CODE BEGIN SPI1_MspInit 1 */

  /* USER CODE END SPI1_MspInit 1 */
  }

}

/**
* @brief SPI MSP De-Initialization
* This function freeze the hardware resources used in this example
* @param hspi: SPI handle pointer
* @retval None
*/
void HAL_SPI_MspDeInit(SPI_HandleTypeDef* hspi)
{
  if(hspi->Instance==SPI1)
  {
  /* USER CODE BEGIN SPI1_MspDeInit 0 */

  /* USER CODE END SPI1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_SPI1_CLK_DISABLE();

    /**SPI1 GPIO Configuration
    PB5     ------> SPI1_MOSI
    PG9     ------> SPI1_MISO
    PA5     ------> SPI1_SCK
    */
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_5);

    HAL_GPIO_DeInit(GPIOG, GPIO_PIN_9);

    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_5);

  /* USER CODE BEGIN SPI1_MspDeInit 1 */

  /* USER CODE END SPI1_MspDeInit 1 */
  }

}
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
