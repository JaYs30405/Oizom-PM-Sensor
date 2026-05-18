/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

UART_HandleTypeDef huart1;
DMA_HandleTypeDef hdma_usart1_tx;

/* USER CODE BEGIN PV */
uint16_t adc_buffer[ADC_BUF_SIZE];
uint8_t uart_tx_buf[3*(ADC_BUF_SIZE/2)];   // byte buffer for UART DMA
volatile uint8_t uart_tx_busy = 0;   // guard flag — don't send if previous not done

int baseline = 0;
int current_peak = 0.0f;
uint16_t peak_index = 0;

volatile uint32_t bin_03 = 0;
volatile uint32_t bin_05 = 0;
volatile uint32_t bin_10 = 0;
volatile uint32_t bin_25 = 0;
volatile uint32_t bin_50 = 0;
volatile uint32_t bin_100 = 0;

volatile uint32_t particles_03  = 0;
volatile uint32_t particles_05  = 0;
volatile uint32_t particles_10  = 0;
volatile uint32_t particles_25  = 0;
volatile uint32_t particles_50  = 0;
volatile uint32_t particles_100 = 0;

volatile int final_peak=0;
volatile int hist[101];

volatile uint32_t ms_counter = 0;

volatile bool ADC_ConvHalfCplt = 0;
volatile bool ADC_ConvCplt = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_ADC1_Init(void);
static void MX_USART1_UART_Init(void);
/* USER CODE BEGIN PFP */
void process_samples(uint16_t *data, uint16_t len);
void log_peak(float peak);
void classify_particle(uint16_t peak);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
	HAL_ADCEx_Calibration_Start(&hadc1);
	HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_buffer, ADC_BUF_SIZE);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
		if(ADC_ConvHalfCplt)
		{
			ADC_ConvHalfCplt = 0;
			process_samples(&adc_buffer[0], ADC_BUF_SIZE/2);			
		}
		else if(ADC_ConvCplt)
		{
			ADC_ConvCplt = 0;
			process_samples(&adc_buffer[ADC_BUF_SIZE/2], ADC_BUF_SIZE/2);
		}
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL6;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV2;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.ContinuousConvMode = ENABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_4;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

	//adc sample time calculation :
	// 12000000 clk freq / (12.5 sar tconv + sampling time)
	
  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 921600;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);
  /* DMA1_Channel4_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel4_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel4_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin : LED_Pin */
  GPIO_InitStruct.Pin = LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* ------------------- DMA Callbacks ------------------- */

void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* hadc)
{
    if(hadc->Instance == ADC1)
    {
			ADC_ConvHalfCplt = 1;
      //process_samples(&adc_buffer[0], ADC_BUF_SIZE/2);
    }
		
//		/* Send first half of adc_buffer over UART DMA */
//		if(!uart_tx_busy)
//		{
//				uart_tx_busy = 1;
//				/* Pack uint16 as two bytes each — MSB first */
//				for(int i = 0; i < ADC_BUF_SIZE/2; i++)
//				{
//							uart_tx_buf[i*3]   = 0xFF;
//							uart_tx_buf[i*3+1] = (adc_buffer[i] >> 8) & 0x0F;
//							uart_tx_buf[i*3+2] =  adc_buffer[i] & 0xFF;
//				}
//				HAL_UART_Transmit_DMA(&huart1, uart_tx_buf, 3*(ADC_BUF_SIZE/2));
//		}
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
    if(hadc->Instance == ADC1)
    {
			ADC_ConvCplt = 1;
        //process_samples(&adc_buffer[ADC_BUF_SIZE/2], ADC_BUF_SIZE/2);
			
//			  /* Send second half of adc_buffer over UART DMA */
//        if(!uart_tx_busy)
//        {
//            uart_tx_busy = 1;
//            for(int i = 0; i < ADC_BUF_SIZE/2; i++)
//            {
//                uart_tx_buf[i*3]   = 0xFF;
//                uart_tx_buf[i*3+1] = (adc_buffer[ADC_BUF_SIZE/2 + i] >> 8) & 0x0F;
//                uart_tx_buf[i*3+2] =  adc_buffer[ADC_BUF_SIZE/2 + i] & 0xFF;
//            }
//            HAL_UART_Transmit_DMA(&huart1, uart_tx_buf, 3*(ADC_BUF_SIZE/2));
//        }
				
			  HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_buffer, ADC_BUF_SIZE);

    }
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if(huart->Instance == USART1)
        uart_tx_busy = 0;
}

/* ------------------- Processing Function ------------------- */

void process_samples(uint16_t *data, uint16_t len)
{
    static int prev_sample = 0;
    static int rising = 0;

    for(uint16_t i = 0; i < len; i++)
    {
        int sample = data[i];

        /* -------- Baseline tracking -------- */

//				if(abs(sample - (int)baseline) < THRESHOLD_LOW)
//				{
//						baseline = 15;//baseline + BASELINE_ALPHA * (sample - baseline);
//				}
					baseline = 15;
					sample = sample - baseline;
					int delta = sample - prev_sample;
        /* -------- Rising detection -------- */

        if(delta >= 0)
        {
            rising++;

            if(sample > current_peak)
                current_peak = sample;
        }
        else
        {
            /* -------- Falling detected ? final peak -------- */

            if(rising > 6)
            {
								final_peak  = current_peak;
								classify_particle(final_peak);
								
								if(final_peak > 4000)
									final_peak = 4000;
								
								hist[final_peak/40]++;
            }

            rising = 0;
            current_peak = 0;
        }

        prev_sample = sample;
    }
}

/* ------------------- Classify Peaks ------------------- */

void classify_particle(uint16_t peak)
{
    if(peak > 550)
        bin_100++;

    else if(peak > 450)
        bin_50++;

    else if(peak > 400)
        bin_25++;

    else if(peak > 350)
        bin_10++;

    else if(peak > 150)
        bin_05++;

    else if(peak > 81)
        bin_03++;
}

/* ------------------- Find Concentration ------------------- */


float particle_concentration(float diameter, uint32_t N)
{
    float volume_ratio = N / FLOW_VOLUME;

    float conc = (PI/6.0f) * RHO * diameter * diameter * diameter * volume_ratio;

    return conc * 1e9f;   // g/m
}

/* ------------------- debug ------------------- */


/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
