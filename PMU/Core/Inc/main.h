/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f7xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define USB_OTG_HS_ULPI_RSTN_Pin GPIO_PIN_13
#define USB_OTG_HS_ULPI_RSTN_GPIO_Port GPIOC
#define CFF0_A1_Pin GPIO_PIN_0
#define CFF0_A1_GPIO_Port GPIOF
#define CFF0_A0_Pin GPIO_PIN_1
#define CFF0_A0_GPIO_Port GPIOF
#define CFF0_EN_Pin GPIO_PIN_2
#define CFF0_EN_GPIO_Port GPIOF
#define CCOMP2_A0_Pin GPIO_PIN_4
#define CCOMP2_A0_GPIO_Port GPIOF
#define CCOMP2_A1_Pin GPIO_PIN_5
#define CCOMP2_A1_GPIO_Port GPIOF
#define CCOMP2_EN_Pin GPIO_PIN_6
#define CCOMP2_EN_GPIO_Port GPIOF
#define CCOMP0_A0_Pin GPIO_PIN_8
#define CCOMP0_A0_GPIO_Port GPIOF
#define CCOMP0_A1_Pin GPIO_PIN_9
#define CCOMP0_A1_GPIO_Port GPIOF
#define CCOMP0_EN_Pin GPIO_PIN_10
#define CCOMP0_EN_GPIO_Port GPIOF
#define PMU_CS_Pin GPIO_PIN_4
#define PMU_CS_GPIO_Port GPIOA
#define CFF2_A0_Pin GPIO_PIN_11
#define CFF2_A0_GPIO_Port GPIOF
#define CFF2_A1_Pin GPIO_PIN_12
#define CFF2_A1_GPIO_Port GPIOF
#define EEP_WP_Pin GPIO_PIN_13
#define EEP_WP_GPIO_Port GPIOF
#define CFF2_EN_Pin GPIO_PIN_0
#define CFF2_EN_GPIO_Port GPIOG
#define CFF3_A1_Pin GPIO_PIN_1
#define CFF3_A1_GPIO_Port GPIOG
#define CFF3_A0_Pin GPIO_PIN_8
#define CFF3_A0_GPIO_Port GPIOE
#define CFF3_EN_Pin GPIO_PIN_9
#define CFF3_EN_GPIO_Port GPIOE
#define CPOH1_Pin GPIO_PIN_10
#define CPOH1_GPIO_Port GPIOE
#define ECC_ENABLE_Pin GPIO_PIN_12
#define ECC_ENABLE_GPIO_Port GPIOE
#define ADC_RESETn_Pin GPIO_PIN_13
#define ADC_RESETn_GPIO_Port GPIOE
#define PMU_RESETn_Pin GPIO_PIN_14
#define PMU_RESETn_GPIO_Port GPIOE
#define BUSYn_Pin GPIO_PIN_8
#define BUSYn_GPIO_Port GPIOD
#define TMP_ARALMn_Pin GPIO_PIN_9
#define TMP_ARALMn_GPIO_Port GPIOD
#define TMP_ARALMn_EXTI_IRQn EXTI9_5_IRQn
#define CPOH0_Pin GPIO_PIN_10
#define CPOH0_GPIO_Port GPIOD
#define CPOL0_Pin GPIO_PIN_11
#define CPOL0_GPIO_Port GPIOD
#define CPOL1_Pin GPIO_PIN_12
#define CPOL1_GPIO_Port GPIOD
#define CGALMn_Pin GPIO_PIN_13
#define CGALMn_GPIO_Port GPIOD
#define CGALMn_EXTI_IRQn EXTI15_10_IRQn
#define LOADn_Pin GPIO_PIN_2
#define LOADn_GPIO_Port GPIOG
#define CPOL2_Pin GPIO_PIN_3
#define CPOL2_GPIO_Port GPIOG
#define CPOH2_Pin GPIO_PIN_4
#define CPOH2_GPIO_Port GPIOG
#define CPOL3_Pin GPIO_PIN_5
#define CPOL3_GPIO_Port GPIOG
#define CPOH3_Pin GPIO_PIN_6
#define CPOH3_GPIO_Port GPIOG
#define ADC_DRDYn_Pin GPIO_PIN_7
#define ADC_DRDYn_GPIO_Port GPIOG
#define ADC_DONEn_Pin GPIO_PIN_8
#define ADC_DONEn_GPIO_Port GPIOG
#define ADC_DONEn_EXTI_IRQn EXTI9_5_IRQn
#define CCOMP3_A1_Pin GPIO_PIN_0
#define CCOMP3_A1_GPIO_Port GPIOD
#define CCOMP3_A0_Pin GPIO_PIN_1
#define CCOMP3_A0_GPIO_Port GPIOD
#define CCOMP1_EN_Pin GPIO_PIN_2
#define CCOMP1_EN_GPIO_Port GPIOD
#define CCOMP1_A0_Pin GPIO_PIN_4
#define CCOMP1_A0_GPIO_Port GPIOD
#define CCOMP3_EN_Pin GPIO_PIN_5
#define CCOMP3_EN_GPIO_Port GPIOD
#define CCOMP1_A1_Pin GPIO_PIN_6
#define CCOMP1_A1_GPIO_Port GPIOD
#define SPI_WORD_SIZE_Pin GPIO_PIN_9
#define SPI_WORD_SIZE_GPIO_Port GPIOG
#define CFF1_EN_Pin GPIO_PIN_10
#define CFF1_EN_GPIO_Port GPIOG
#define CFF1_A1_Pin GPIO_PIN_12
#define CFF1_A1_GPIO_Port GPIOG
#define SPI_MODE_Pin GPIO_PIN_13
#define SPI_MODE_GPIO_Port GPIOG
#define CFF1_A0_Pin GPIO_PIN_14
#define CFF1_A0_GPIO_Port GPIOG
#define ADC_CS_Pin GPIO_PIN_9
#define ADC_CS_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
