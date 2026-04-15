/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    gpio.c
  * @brief   This file provides code for the configuration
  *          of all used GPIO pins.
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

/* Includes ------------------------------------------------------------------*/
#include "gpio.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/*----------------------------------------------------------------------------*/
/* Configure GPIO                                                             */
/*----------------------------------------------------------------------------*/
/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/** Configure pins as
        * Analog
        * Input
        * Output
        * EVENT_OUT
        * EXTI
*/
void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(USB_OTG_HS_ULPI_RSTN_GPIO_Port, USB_OTG_HS_ULPI_RSTN_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOF, CFF0_A1_Pin|CFF0_A0_Pin|CFF0_EN_Pin|CCOMP2_A0_Pin
                          |CCOMP2_A1_Pin|CCOMP2_EN_Pin|CCOMP0_A0_Pin|CCOMP0_A1_Pin
                          |CCOMP0_EN_Pin|CFF2_A0_Pin|CFF2_A1_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(PMU_CS_GPIO_Port, PMU_CS_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(EEP_WP_GPIO_Port, EEP_WP_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOG, CFF2_EN_Pin|CFF3_A1_Pin|ADC_DRDYn_Pin|CFF1_EN_Pin
                          |CFF1_A1_Pin|SPI_MODE_Pin|CFF1_A0_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOE, CFF3_A0_Pin|CFF3_EN_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOE, ECC_ENABLE_Pin|ADC_RESETn_Pin|PMU_RESETn_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOG, LOADn_Pin|SPI_WORD_SIZE_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, CCOMP3_A1_Pin|CCOMP3_A0_Pin|CCOMP1_EN_Pin|CCOMP1_A0_Pin
                          |CCOMP3_EN_Pin|CCOMP1_A1_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(ADC_CS_GPIO_Port, ADC_CS_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin : USB_OTG_HS_ULPI_RSTN_Pin */
  GPIO_InitStruct.Pin = USB_OTG_HS_ULPI_RSTN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(USB_OTG_HS_ULPI_RSTN_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : CFF0_A1_Pin CFF0_A0_Pin CFF0_EN_Pin CCOMP2_A0_Pin
                           CCOMP2_A1_Pin CCOMP2_EN_Pin CCOMP0_A0_Pin CCOMP0_A1_Pin
                           CCOMP0_EN_Pin CFF2_A0_Pin CFF2_A1_Pin EEP_WP_Pin */
  GPIO_InitStruct.Pin = CFF0_A1_Pin|CFF0_A0_Pin|CFF0_EN_Pin|CCOMP2_A0_Pin
                          |CCOMP2_A1_Pin|CCOMP2_EN_Pin|CCOMP0_A0_Pin|CCOMP0_A1_Pin
                          |CCOMP0_EN_Pin|CFF2_A0_Pin|CFF2_A1_Pin|EEP_WP_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  /*Configure GPIO pin : PMU_CS_Pin */
  GPIO_InitStruct.Pin = PMU_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(PMU_CS_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : CFF2_EN_Pin CFF3_A1_Pin LOADn_Pin ADC_DRDYn_Pin
                           SPI_WORD_SIZE_Pin CFF1_EN_Pin CFF1_A1_Pin SPI_MODE_Pin
                           CFF1_A0_Pin */
  GPIO_InitStruct.Pin = CFF2_EN_Pin|CFF3_A1_Pin|LOADn_Pin|ADC_DRDYn_Pin
                          |SPI_WORD_SIZE_Pin|CFF1_EN_Pin|CFF1_A1_Pin|SPI_MODE_Pin
                          |CFF1_A0_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  /*Configure GPIO pins : CFF3_A0_Pin CFF3_EN_Pin ECC_ENABLE_Pin ADC_RESETn_Pin
                           PMU_RESETn_Pin */
  GPIO_InitStruct.Pin = CFF3_A0_Pin|CFF3_EN_Pin|ECC_ENABLE_Pin|ADC_RESETn_Pin
                          |PMU_RESETn_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pin : CPOH1_Pin */
  GPIO_InitStruct.Pin = CPOH1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(CPOH1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : BUSYn_Pin CPOH0_Pin CPOL0_Pin CPOL1_Pin */
  GPIO_InitStruct.Pin = BUSYn_Pin|CPOH0_Pin|CPOL0_Pin|CPOL1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pins : TMP_ARALMn_Pin CGALMn_Pin */
  GPIO_InitStruct.Pin = TMP_ARALMn_Pin|CGALMn_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pins : CPOL2_Pin CPOH2_Pin CPOL3_Pin CPOH3_Pin */
  GPIO_InitStruct.Pin = CPOL2_Pin|CPOH2_Pin|CPOL3_Pin|CPOH3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  /*Configure GPIO pin : ADC_DONEn_Pin */
  GPIO_InitStruct.Pin = ADC_DONEn_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(ADC_DONEn_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : CCOMP3_A1_Pin CCOMP3_A0_Pin CCOMP1_EN_Pin CCOMP1_A0_Pin
                           CCOMP3_EN_Pin CCOMP1_A1_Pin */
  GPIO_InitStruct.Pin = CCOMP3_A1_Pin|CCOMP3_A0_Pin|CCOMP1_EN_Pin|CCOMP1_A0_Pin
                          |CCOMP3_EN_Pin|CCOMP1_A1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pin : ADC_CS_Pin */
  GPIO_InitStruct.Pin = ADC_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(ADC_CS_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

}

/* USER CODE BEGIN 2 */

/* USER CODE END 2 */
