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
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
#include "stdlib.h"
#include "stdbool.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
extern volatile uint32_t particles_03 ;
extern volatile uint32_t particles_05 ;
extern volatile uint32_t particles_10 ;
extern volatile uint32_t particles_25 ;
extern volatile uint32_t particles_50 ;
extern volatile uint32_t particles_100;

extern volatile uint32_t bin_03;
extern volatile uint32_t bin_05;
extern volatile uint32_t bin_10;
extern volatile uint32_t bin_25;
extern volatile uint32_t bin_50;
extern volatile uint32_t bin_100;
extern volatile uint32_t ms_counter;
extern volatile uint8_t one_second_flag;
extern volatile int hist[101];
/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */
#define ADC_BUF_SIZE      256
#define BASELINE_ALPHA    0.001f
#define THRESHOLD_HIGH    50
#define THRESHOLD_LOW     20
#define MAX_LOG           100
#define PI 3.1415926f
#define RHO 1650.0f
#define FLOW_VOLUME 9.17e-6f
/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
float particle_concentration(float diameter, uint32_t N);

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LED_Pin GPIO_PIN_13
#define LED_GPIO_Port GPIOC

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
