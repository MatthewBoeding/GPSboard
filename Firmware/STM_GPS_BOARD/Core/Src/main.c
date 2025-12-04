/* USER CODE BEGIN Header */
// Author: Paul Scalise, adapted from Matthew Boeding's work
// This program will send the GPS coordinates every 4 seconds via CAN.
// Intended for the GPS board designed for ECEN 435 at the University of Nebraska-Lincoln
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
#include <stdbool.h>
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
CAN_HandleTypeDef hcan;

TIM_HandleTypeDef htim14;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
uint8_t uart_rx_buf[128] = {0};
volatile bool measure_flag = false;
uint8_t frameBuffer[100];
uint8_t lat[8];
uint8_t lon[8];
uint8_t fixedlat[8] = {"LA00.00N"};
uint8_t fixedlon[8] = {"LO000.0E"};

uint8_t gpsOn[8]       = {0xB5,0x62,0x06,0x57,0x00,0x00,0x5D,0x1D};
uint8_t pwrControl[16] = {0xB5,0x62,0x06,0x86,0x08,0x00,0x00,0x03,0x00,0x00,0x00,0x00,0x00,0x00,0x97,0x6F};
uint8_t pwrControl2[8] = {0xB5,0x62,0x06,0x86,0x00,0x00,0x8C,0xAA};
uint8_t coldRestart[12]= {0xB5,0x62,0x06,0x04,0x04,0x00,0xFF,0xFF,0x02,0x00,0x0E,0x61};
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_CAN_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_TIM14_Init(void);
/* USER CODE BEGIN PFP */
static void gpsCommand(const uint8_t *d, uint8_t len);
static uint8_t uartRecv(void);
static void uartWrite(uint8_t b);
static uint8_t getFrame(void);
static void processFrame(uint8_t size);
static void CAN_Send8(const uint8_t *payload8, uint16_t std_id);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == TIM14) {
    measure_flag = true;
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
  MX_CAN_Init();
  MX_USART2_UART_Init();
  MX_TIM14_Init();
  /* USER CODE BEGIN 2 */

  HAL_CAN_Start(&hcan);

  // Configure an accept-all CAN filter into FIFO0
  CAN_FilterTypeDef f = {0};
  f.FilterBank = 0;
  f.FilterMode = CAN_FILTERMODE_IDMASK;
  f.FilterScale = CAN_FILTERSCALE_32BIT;
  f.FilterIdHigh = 0x0000;
  f.FilterIdLow  = 0x0000;
  f.FilterMaskIdHigh = 0x0000;
  f.FilterMaskIdLow  = 0x0000;
  f.FilterFIFOAssignment = CAN_FILTER_FIFO0;
  f.FilterActivation = ENABLE;
  HAL_CAN_ConfigFilter(&hcan, &f);

  // 4s interrupt to get new coordinates
  HAL_TIM_Base_Start_IT(&htim14);

  // Initialize GPS (MAX-M8C-0)
  gpsCommand(gpsOn,       sizeof(gpsOn));
  gpsCommand(pwrControl,  sizeof(pwrControl));
  gpsCommand(pwrControl2, sizeof(pwrControl2));
  gpsCommand(coldRestart, sizeof(coldRestart));
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1) {
    if (measure_flag) {
      measure_flag = false;

      uint8_t n = getFrame();
      if (n > 0) {
        processFrame(n);
        // Send two 8-byte frames with the lat/lon strings
        CAN_Send8(lat, 0x101);
        CAN_Send8(lon, 0x101);
      }
    }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI48;
  RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI48;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief CAN Initialization Function
  * @param None
  * @retval None
  */
static void MX_CAN_Init(void)
{

  /* USER CODE BEGIN CAN_Init 0 */

  /* USER CODE END CAN_Init 0 */

  /* USER CODE BEGIN CAN_Init 1 */

  /* USER CODE END CAN_Init 1 */
  hcan.Instance = CAN;
  hcan.Init.Prescaler = 120;
  hcan.Init.Mode = CAN_MODE_NORMAL;
  hcan.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan.Init.TimeSeg1 = CAN_BS1_2TQ;
  hcan.Init.TimeSeg2 = CAN_BS2_1TQ;
  hcan.Init.TimeTriggeredMode = DISABLE;
  hcan.Init.AutoBusOff = DISABLE;
  hcan.Init.AutoWakeUp = DISABLE;
  hcan.Init.AutoRetransmission = DISABLE;
  hcan.Init.ReceiveFifoLocked = DISABLE;
  hcan.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CAN_Init 2 */

  /* USER CODE END CAN_Init 2 */

}

/**
  * @brief TIM14 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM14_Init(void)
{

  /* USER CODE BEGIN TIM14_Init 0 */

  /* USER CODE END TIM14_Init 0 */

  /* USER CODE BEGIN TIM14_Init 1 */

  /* USER CODE END TIM14_Init 1 */
  htim14.Instance = TIM14;
  htim14.Init.Prescaler = 47999;
  htim14.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim14.Init.Period = 3999;
  htim14.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim14.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim14) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM14_Init 2 */

  /* USER CODE END TIM14_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 9600;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
static void gpsCommand(const uint8_t *d, uint8_t len)
{
  HAL_UART_Transmit(&huart2, (uint8_t*)d, len, HAL_MAX_DELAY);
}

static void uartWrite(uint8_t b)
{
  HAL_UART_Transmit(&huart2, &b, 1, HAL_MAX_DELAY);
}

__inline static uint8_t uartRecv(void)
{
  uint8_t b;
  HAL_UART_Receive(&huart2, &b, 1, 100);
  return b;
}

static uint8_t getFrame(void)
{
  uint8_t frameSize = 0;
  bool receive = true, gotHeader = false;
  uint8_t hdr[5];

  while (receive) {
    while (!gotHeader) {
      uint8_t c = uartRecv();
      if (c == '$') {
        for (int i = 0; i < 5; i++){
        	hdr[i] = uartRecv();
        }
        gotHeader = (hdr[0]=='G' && hdr[1]=='N' && hdr[2]=='G' && hdr[3]=='L' && hdr[4]=='L');
      }
    }
    uint8_t c = uartRecv();
    if (c == '$') {
      receive = false;
    } else {
      if (frameSize < sizeof(frameBuffer)) frameBuffer[frameSize++] = c;
    }
  }
  return frameSize;
}

static void processFrame(uint8_t size)
{
  uint8_t commas[7] = {0};
  int i = 0, j = 0;

  while (i < 7 && j < size) {
      if (frameBuffer[j] == ',') {
          commas[i++] = j;
      }
      j++;
  }

  if (i < 6 || frameBuffer[commas[5] + 1] == 'V' || (commas[1] - commas[0] < 2)) {
    memcpy(lat, fixedlat, sizeof(lat));
    memcpy(lon, fixedlon, sizeof(lon));
    return;
  }

  // LAT: "LA00.00N" from ddmm.mmmm,N/S
  lat[0]='L'; lat[1]='A';
  lat[2] = frameBuffer[commas[0] + 1];
  lat[3] = frameBuffer[commas[0] + 2];
  lat[4] = '.';
  lat[5] = frameBuffer[commas[0] + 3];
  lat[6] = frameBuffer[commas[0] + 4];
  lat[7] = frameBuffer[commas[1] + 1];

  // LON: "LO000.0E" from dddmm.mmmm,E/W
  lon[0]='L'; lon[1]='O';
  lon[2] = frameBuffer[commas[2] + 1];
  lon[3] = frameBuffer[commas[2] + 2];
  lon[4] = frameBuffer[commas[2] + 3];
  lon[5] = '.';
  lon[6] = frameBuffer[commas[2] + 4];
  lon[7] = frameBuffer[commas[3] + 1];
}

static void CAN_Send8(const uint8_t *payload8, uint16_t std_id)
{
  CAN_TxHeaderTypeDef hdr;
  hdr.StdId = std_id;
  hdr.ExtId = 0;
  hdr.IDE   = CAN_ID_STD;
  hdr.RTR   = CAN_RTR_DATA;
  hdr.DLC   = 8;
  hdr.TransmitGlobalTime = DISABLE;

  uint32_t mb;
  HAL_CAN_AddTxMessage(&hcan, &hdr, (uint8_t*)payload8, &mb);
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  __disable_irq();
  while (1) { }
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
  (void)file; (void)line;
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
