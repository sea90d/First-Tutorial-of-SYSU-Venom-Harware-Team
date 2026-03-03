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
#include "oled.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

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

I2C_HandleTypeDef hi2c1;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC1_Init(void);
static void MX_I2C1_Init(void);
static void MX_USART1_UART_Init(void);
/* USER CODE BEGIN PFP */
static HAL_StatusTypeDef AHT20_Init(void);
static HAL_StatusTypeDef AHT20_Read(float *temperature, float *humidity);
static float Read_ADC_Voltage(void);
static float Read_NTC_Temperature(void);
static float Calculate_NTC_Temperature(float voltage);
static void UART_PrintString(const char *str);
static void I2C1_ScanDevices(void);
static void OLED_ShowEnvData(uint8_t aht20_valid, float temp_aht20, float hum_aht20, float temp_ntc, float temp_avg);
static void UART_PrintEnvData(uint8_t aht20_valid, float temp_aht20, float hum_aht20, float temp_ntc, float temp_avg);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
#define AHT20_I2C_ADDR              (0x38U << 1)
#define AHT20_CMD_SOFT_RESET        0xBA
#define AHT20_CMD_INIT_1            0xBE
#define AHT20_CMD_MEASURE_1         0xAC
#define AHT20_CMD_MEASURE_2         0x33
#define AHT20_CMD_MEASURE_3         0x00
#define AHT20_STATUS_BUSY_MASK      0x80
#define AHT20_STATUS_CAL_MASK       0x08

#define ADC_VREF                    3.3f
#define ADC_MAX_VALUE               4095.0f
#define NTC_REF_RESISTOR            10000.0f
#define NTC_MIN_VALID_VOLTAGE       0.01f

static void UART_PrintString(const char *str)
{
  if (str == NULL)
  {
    return;
  }

  (void)HAL_UART_Transmit(&huart1, (uint8_t *)str, (uint16_t)strlen(str), 100);
}

static void I2C1_ScanDevices(void)
{
  char tx_buf[80] = {0};
  uint8_t found_count = 0;

  UART_PrintString("\r\n[I2C1] Scan start...\r\n");

  for (uint8_t addr = 1; addr < 0x80; addr++)
  {
    if (HAL_I2C_IsDeviceReady(&hi2c1, (uint16_t)(addr << 1), 2, 20) == HAL_OK)
    {
      int len = snprintf(tx_buf, sizeof(tx_buf), "[I2C1] Found: 7-bit 0x%02X, 8-bit W 0x%02X\r\n", addr, addr << 1);
      if (len > 0)
      {
        (void)HAL_UART_Transmit(&huart1, (uint8_t *)tx_buf, (uint16_t)len, 100);
      }
      found_count++;
    }
  }

  if (found_count == 0U)
  {
    UART_PrintString("[I2C1] No device found.\r\n");
  }
  else
  {
    int len = snprintf(tx_buf, sizeof(tx_buf), "[I2C1] Scan done, total %u device(s).\r\n", found_count);
    if (len > 0)
    {
      (void)HAL_UART_Transmit(&huart1, (uint8_t *)tx_buf, (uint16_t)len, 100);
    }
  }
}

static HAL_StatusTypeDef AHT20_Init(void)
{
  uint8_t reset_cmd = AHT20_CMD_SOFT_RESET;
  uint8_t init_cmd[3] = {AHT20_CMD_INIT_1, 0x08, 0x00};
  uint8_t status = 0;

  (void)HAL_I2C_Master_Transmit(&hi2c1, AHT20_I2C_ADDR, &reset_cmd, 1, 100);
  HAL_Delay(20);

  if (HAL_I2C_Master_Transmit(&hi2c1, AHT20_I2C_ADDR, init_cmd, sizeof(init_cmd), 100) != HAL_OK)
  {
    return HAL_ERROR;
  }
  HAL_Delay(10);

  if (HAL_I2C_Master_Receive(&hi2c1, AHT20_I2C_ADDR, &status, 1, 100) != HAL_OK)
  {
    return HAL_ERROR;
  }

  return ((status & AHT20_STATUS_CAL_MASK) != 0U) ? HAL_OK : HAL_ERROR;
}

static HAL_StatusTypeDef AHT20_Read(float *temperature, float *humidity)
{
  uint8_t trigger_cmd[3] = {AHT20_CMD_MEASURE_1, AHT20_CMD_MEASURE_2, AHT20_CMD_MEASURE_3};
  uint8_t rx_buf[7] = {0};
  uint32_t raw_h = 0;
  uint32_t raw_t = 0;

  if ((temperature == NULL) || (humidity == NULL))
  {
    return HAL_ERROR;
  }

  if (HAL_I2C_Master_Transmit(&hi2c1, AHT20_I2C_ADDR, trigger_cmd, sizeof(trigger_cmd), 100) != HAL_OK)
  {
    return HAL_ERROR;
  }

  HAL_Delay(80);

  for (uint8_t retry = 0; retry < 5; retry++)
  {
    if (HAL_I2C_Master_Receive(&hi2c1, AHT20_I2C_ADDR, rx_buf, sizeof(rx_buf), 100) != HAL_OK)
    {
      return HAL_ERROR;
    }

    if ((rx_buf[0] & AHT20_STATUS_BUSY_MASK) == 0U)
    {
      raw_h = ((uint32_t)rx_buf[1] << 12) | ((uint32_t)rx_buf[2] << 4) | ((uint32_t)rx_buf[3] >> 4);
      raw_t = (((uint32_t)rx_buf[3] & 0x0FU) << 16) | ((uint32_t)rx_buf[4] << 8) | rx_buf[5];

      *humidity = ((float)raw_h * 100.0f) / 1048576.0f;
      *temperature = ((float)raw_t * 200.0f) / 1048576.0f - 50.0f;
      return HAL_OK;
    }

    HAL_Delay(5);
  }

  return HAL_BUSY;
}

static float Read_ADC_Voltage(void)
{
  uint32_t adc_raw = 0;

  if (HAL_ADC_Start(&hadc1) != HAL_OK)
  {
    return 0.0f;
  }

  if (HAL_ADC_PollForConversion(&hadc1, 50) != HAL_OK)
  {
    (void)HAL_ADC_Stop(&hadc1);
    return 0.0f;
  }

  adc_raw = HAL_ADC_GetValue(&hadc1);
  (void)HAL_ADC_Stop(&hadc1);

  return ((float)adc_raw * ADC_VREF) / ADC_MAX_VALUE;
}

static float Calculate_NTC_Temperature(float voltage)
{
  float resistance = 0.0f;
  float steinhart = 0.0f;

  if ((voltage <= NTC_MIN_VALID_VOLTAGE) || (voltage >= (ADC_VREF - NTC_MIN_VALID_VOLTAGE)))
  {
    return NAN;
  }

  resistance = (ADC_VREF - voltage) * NTC_REF_RESISTOR / voltage;
  steinhart = resistance / NTC_REF_RESISTOR;
  steinhart = logf(steinhart);
  steinhart = 1.0f / (0.001129148f + 0.000234125f * steinhart + 0.0000000876741f * powf(steinhart, 3.0f));

  return steinhart - 273.15f;
}

static float Read_NTC_Temperature(void)
{
  float voltage_sum = 0.0f;

  for (uint8_t i = 0; i < 8; i++)
  {
    voltage_sum += Read_ADC_Voltage();
    HAL_Delay(2);
  }

  return Calculate_NTC_Temperature(voltage_sum / 8.0f);
}

static void OLED_ShowEnvData(uint8_t aht20_valid, float temp_aht20, float hum_aht20, float temp_ntc, float temp_avg)
{
  char line[24] = {0};

  OLED_NewFrame();

  if (aht20_valid != 0U)
  {
    snprintf(line, sizeof(line), "T_AHT20:%5.2fC", temp_aht20);
  }
  else
  {
    snprintf(line, sizeof(line), "T_AHT20:  ERR ");
  }
  OLED_PrintASCIIString(0, 0, line, &afont12x6, OLED_COLOR_NORMAL);

  if (!isnan(temp_ntc))
  {
    snprintf(line, sizeof(line), "T_NTC :%5.2fC", temp_ntc);
  }
  else
  {
    snprintf(line, sizeof(line), "T_NTC :  ERR ");
  }
  OLED_PrintASCIIString(0, 16, line, &afont12x6, OLED_COLOR_NORMAL);

  if (!isnan(temp_avg))
  {
    snprintf(line, sizeof(line), "T_AVG :%5.2fC", temp_avg);
  }
  else
  {
    snprintf(line, sizeof(line), "T_AVG :  ERR ");
  }
  OLED_PrintASCIIString(0, 32, line, &afont12x6, OLED_COLOR_NORMAL);

  if (aht20_valid != 0U)
  {
    snprintf(line, sizeof(line), "HUM   :%5.2f%%", hum_aht20);
  }
  else
  {
    snprintf(line, sizeof(line), "HUM   :  ERR ");
  }
  OLED_PrintASCIIString(0, 48, line, &afont12x6, OLED_COLOR_NORMAL);

  OLED_ShowFrame();
}

static void UART_PrintEnvData(uint8_t aht20_valid, float temp_aht20, float hum_aht20, float temp_ntc, float temp_avg)
{
  char tx_buf[128] = {0};
  int len = 0;

  if ((aht20_valid != 0U) && !isnan(temp_ntc) && !isnan(temp_avg))
  {
    len = snprintf(tx_buf, sizeof(tx_buf),
                   "AHT20 T=%.2f C, H=%.2f %%RH | NTC T=%.2f C | AVG T=%.2f C\r\n",
                   temp_aht20, hum_aht20, temp_ntc, temp_avg);
  }
  else
  {
    len = snprintf(tx_buf, sizeof(tx_buf),
                   "Sensor read error | AHT20=%s | NTC=%s | AVG=%s\r\n",
                   (aht20_valid != 0U) ? "OK" : "ERR",
                   isnan(temp_ntc) ? "ERR" : "OK",
                   isnan(temp_avg) ? "ERR" : "OK");
  }

  if ((aht20_valid == 0U) && !isnan(temp_ntc) && !isnan(temp_avg))
  {
    len = snprintf(tx_buf, sizeof(tx_buf),
                   "AHT20 read error | NTC T=%.2f C | AVG T=%.2f C\r\n",
                   temp_ntc, temp_avg);
  }

  if (len > 0)
  {
    (void)HAL_UART_Transmit(&huart1, (uint8_t *)tx_buf, (uint16_t)len, 100);
  }
}

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
  MX_ADC1_Init();
  MX_I2C1_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  float temp_aht20 = 0.0f;
  float hum_aht20 = 0.0f;
  float temp_ntc = 0.0f;
  float temp_avg = 0.0f;
  uint8_t aht20_valid = 0U;

  I2C1_ScanDevices();

  HAL_Delay(100);
  OLED_Init();

  if (AHT20_Init() == HAL_OK)
  {
    aht20_valid = 1U;
  }

  OLED_ShowEnvData(aht20_valid, temp_aht20, hum_aht20, temp_ntc, temp_avg);
  UART_PrintEnvData(aht20_valid, temp_aht20, hum_aht20, temp_ntc, temp_avg);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    if (AHT20_Read(&temp_aht20, &hum_aht20) == HAL_OK)
    {
      aht20_valid = 1U;
    }
    else
    {
      aht20_valid = 0U;
    }

    temp_ntc = Read_NTC_Temperature();
    temp_avg = (aht20_valid != 0U) ? ((temp_aht20 + temp_ntc) / 2.0f) : temp_ntc;

    OLED_ShowEnvData(aht20_valid, temp_aht20, hum_aht20, temp_ntc, temp_avg);
    UART_PrintEnvData(aht20_valid, temp_aht20, hum_aht20, temp_ntc, temp_avg);

    HAL_Delay(1000);
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

  /** Configure the main internal regulator output voltage
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
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

  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc1.Init.LowPowerAutoWait = DISABLE;
  hadc1.Init.LowPowerAutoPowerOff = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  hadc1.Init.SamplingTimeCommon1 = ADC_SAMPLETIME_160CYCLES_5;
  hadc1.Init.SamplingTimeCommon2 = ADC_SAMPLETIME_160CYCLES_5;
  hadc1.Init.OversamplingMode = DISABLE;
  hadc1.Init.TriggerFrequencyMode = ADC_TRIGGER_FREQ_HIGH;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLINGTIME_COMMON_1;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x00503D58;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

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
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart1, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart1, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

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
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);

  /*Configure GPIO pin : PA1 */
  GPIO_InitStruct.Pin = GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

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

#ifdef  USE_FULL_ASSERT
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
