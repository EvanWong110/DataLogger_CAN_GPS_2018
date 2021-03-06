/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "can.h"
#include "dma.h"
#include "fatfs.h"
#include "rtc.h"
#include "sdmmc.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "string.h"
#include "stdio.h"

#include "user/config.h"
#include "user/can_receiver.h"
#include "user/action_scheduler.h"
#include "user/file_reading_buffer.h"
#include "user/file_writing_buffer.h"
#include "user/led_driver.h"
#include "user/uart_driver.h"
#include "user/gps_driver.h"
#include "user/do_driver.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define INITIALIZATION_LED_ON	950
#define INITIALIZATION_LED_OFF	50

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

ConfigDataManager_TypeDef				configDataManager;
DataSaver_TypeDef						dataSaver;

RTCDriver_TypeDef						rtcDriver;
GPSDriver_TypeDef						gpsDriver;
volatile CANTransceiverDriver_TypeDef	canTransceiverDriver;
volatile MSTimerDriver_TypeDef			msTimerDriver;
DODriver_TypeDef						gpsResetDriver;
volatile UartDriver_TypeDef				uartGpsDriver;

volatile CANReceiver_TypeDef			canReceiver;

FileSystemWrapper_TypeDef				fileSystem;

LedDriver_TypeDef						ledDebug1Driver;
LedDriver_TypeDef						ledDebug2Driver;

UartReceiverStartTerm_TypeDef			uartGpsNMEAReceiverTerm;
UartReceiverStartLength_TypeDef			uartGpsUBXReceiverTerm;

ActionScheduler_TypeDef					actionScheduler;

LedDriver_Pin_TypeDef ledDebug1Pin	= my_LED_DEBUG1_Pin;
LedDriver_Pin_TypeDef ledDebug2Pin	= my_LED_DEBUG2_Pin;
DODriver_Pin_TypeDef gpsResetPin	= my_GPS_RESET_N_Pin;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_NVIC_Init(void);
/* USER CODE BEGIN PFP */

void FIFOTest(){

	volatile FIFOMultiread_TypeDef						rxFifo = {0};
	volatile UartReceiverStartTerm_FIFOElem_TypeDef		receiveBuffer[128];

	FIFOMultiread_init(&rxFifo, receiveBuffer, sizeof(UartReceiverStartTerm_FIFOElem_TypeDef), 128);
	FIFOMultireadReader_TypeDef id;
	FIFOMultiread_registerReader(&rxFifo, &id);

	UartReceiverStartTerm_FIFOElem_TypeDef byteWithTimestamp;

  for (uint16_t i=0; i<128; i++){
	  byteWithTimestamp.dataByte = i;
	  byteWithTimestamp.msTime = i+128;
	if (FIFOMultiread_enqueue(&rxFifo, (void*) &byteWithTimestamp) != FIFOMultiread_Status_OK){
		Warning_Handler("UartDriver_receivedByteCallback function. FIFOMultiread_enqueue returned error.");
	}
  }

  for (uint8_t i=0; i<128; i++){
  	if (FIFOMultiread_dequeue(&id, (void*) &byteWithTimestamp) != FIFOMultiread_Status_OK){
  		Warning_Handler("UartDriver_receivedByteCallback function. FIFOMultiread_enqueue returned error.");
  	}
  }

  for (uint16_t i=0; i<128; i++){
	  byteWithTimestamp.dataByte = i;
	  byteWithTimestamp.msTime = i+128;
	if (FIFOMultiread_enqueue(&rxFifo, (void*) &byteWithTimestamp) != FIFOMultiread_Status_OK){
		Warning_Handler("UartDriver_receivedByteCallback function. FIFOMultiread_enqueue returned error.");
	}
  }

  for (uint16_t i=0; i<128; i++){
  	if (FIFOMultiread_dequeue(&id, (void*) &byteWithTimestamp) != FIFOMultiread_Status_OK){
  		Warning_Handler("UartDriver_receivedByteCallback function. FIFOMultiread_enqueue returned error.");
  	}
  }

}

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void HAL_SYSTICK_Callback(void){
	if (LedDriver_1msElapsedCallbackHandler(&ledDebug1Driver) != LedDriver_Status_OK){
		Error_Handler();
	}
	if (LedDriver_1msElapsedCallbackHandler(&ledDebug2Driver) != LedDriver_Status_OK){
		Error_Handler();
	}
	/*if (ActionScheduler_1msElapsedCallbackHandler(&actionScheduler) != ActionScheduler_Status_OK){
		Error_Handler();
	}*/
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

#ifdef  USE_FULL_ASSERT
	GPSDriver_UnitTests();
	CANData_UnitTests();
#endif

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_SDMMC1_SD_Init();
  MX_FATFS_Init();

  /* Initialize interrupts */
  MX_NVIC_Init();
  /* USER CODE BEGIN 2 */

  if (LedDriver_init(&ledDebug1Driver, (LedDriver_Port_TypeDef*)my_LED_DEBUG1_GPIO_Port, &ledDebug1Pin) != LedDriver_Status_OK){
	  Error_Handler();
  }
  if (LedDriver_init(&ledDebug2Driver, (LedDriver_Port_TypeDef*)my_LED_DEBUG2_GPIO_Port, &ledDebug2Pin) != LedDriver_Status_OK){
	  Error_Handler();
  }

  if (LedDriver_BlinkingLed(&ledDebug2Driver, INITIALIZATION_LED_ON, INITIALIZATION_LED_OFF) != LedDriver_Status_OK){
	  return ActionScheduler_Status_Error;
  }

  if (RTCDriver_init(&rtcDriver, &hrtc) != RTCDriver_Status_OK){
	  Error_Handler();
  }

  if (MSTimerDriver_init(&msTimerDriver, true) != MSTimerDriver_Status_OK){
	  Error_Handler();
  }

  if (FileSystemWrapper_init(&fileSystem) != FileSystemWrapper_Status_OK){
	  Error_Handler();
  }

  if (ConfigDataManager_init(&configDataManager, &fileSystem) != ConfigDataManager_Status_OK){
	  Error_Handler();
  }

  if (ConfigDataManager_readConfig(&configDataManager) != ConfigDataManager_Status_OK){
	  Error_Handler();
  }

  Config_TypeDef* pConfig;
  if (ConfigDataManager_getConfigPointer(&configDataManager, &pConfig) != ConfigDataManager_Status_OK){
	  Error_Handler();
  }

  if (DataSaver_init(&dataSaver, &configDataManager, &fileSystem) != DataSaver_Status_OK){
	  Error_Handler();
  }

  if (DODriver_init(&gpsResetDriver, (DODriver_Port_TypeDef*)my_GPS_RESET_N_GPIO_Port, &gpsResetPin, false) != DODriver_Status_OK){
	  Error_Handler();
  }

  GPSDriver_Status_TypeDef					retGps		= GPSDriver_Status_OK;
  UartDriver_Status_TypeDef					retUartDrv	= UartDriver_Status_OK;
  UartReceiverStartTerm_Status_TypeDef		retUartRcv	= UartReceiverStartTerm_Status_OK;
  UartReceiverStartLength_Status_TypeDef	retUartRcv2	= UartReceiverStartLength_Status_OK;

  while (pConfig->gpsFrequency != Config_GPSFrequency_OFF){ //< one time loop
	  if ((retUartDrv = UartDriver_init(&uartGpsDriver, &huart1, USART1, &msTimerDriver, 115200)) != UartDriver_Status_OK){
		  Warning_Handler("UartDriver initialization problem.");
		  retGps = GPSDriver_Status_UartDriverError;
		  break;
	  }
	  if ((retUartRcv = UartReceiverStartTerm_init(&uartGpsNMEAReceiverTerm, &uartGpsDriver)) != UartReceiverStartTerm_Status_OK){
		  Warning_Handler("UartReceiver initialization problem.");
		  retGps = GPSDriver_Status_UartReceiverStartTermError;
		  break;
	  }
	  if ((retUartRcv2 = UartReceiverStartLength_init(&uartGpsUBXReceiverTerm, &uartGpsDriver)) != UartReceiverStartLength_Status_OK){
		  Warning_Handler("UartReceiver initialization problem.");
		  retGps = GPSDriver_Status_UartReceiverStartLengthError;
		  break;
	  }
	  if ((retUartDrv = UartDriver_startReceiver(&uartGpsDriver)) != UartDriver_Status_OK){
		  Warning_Handler("UartDriver startReceiver problem.");
		  retGps = GPSDriver_Status_UartDriverError;
		  break;
	  }
	  if ((retGps = GPSDriver_init(&gpsDriver, &uartGpsDriver, &uartGpsNMEAReceiverTerm, &uartGpsUBXReceiverTerm, &msTimerDriver, &gpsResetDriver, pConfig->gpsFrequency)) != GPSDriver_Status_OK){
		  Warning_Handler("GPS initialization problem.");
		  break;
	  }
	  break;
  }

  if (retGps != GPSDriver_Status_OK){
	  if ((retGps = GPSDriver_setOFF(&gpsDriver)) != GPSDriver_Status_OK){
		  Warning_Handler("GPS switching off problem.");
	  }
  }

  if (CANTransceiverDriver_init(&canTransceiverDriver, pConfig, &hcan1, CAN1) != CANTransceiverDriver_Status_OK){
	  Error_Handler();
  }
  if (CANReceiver_init(&canReceiver, pConfig, &canTransceiverDriver, &msTimerDriver) != CANReceiver_Status_OK){
	  Error_Handler();
  }

  if (ActionScheduler_init(&actionScheduler, &configDataManager, &dataSaver, &canReceiver, &gpsDriver, &msTimerDriver, &rtcDriver, &ledDebug2Driver, &ledDebug1Driver) != ActionScheduler_Status_OK){
	  Error_Handler();
  }
  if (ActionScheduler_startScheduler(&actionScheduler) != ActionScheduler_Status_OK){
	  Error_Handler();
  }

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

	if (ActionScheduler_thread(&actionScheduler) != ActionScheduler_Status_OK){
		Warning_Handler("ActionScheduler_thread returned error.");
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
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);
  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE|RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 96;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Activate the Over-Drive mode
  */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC|RCC_PERIPHCLK_USART1
                              |RCC_PERIPHCLK_SDMMC1|RCC_PERIPHCLK_CLK48;
  PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
  PeriphClkInitStruct.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
  PeriphClkInitStruct.Clk48ClockSelection = RCC_CLK48SOURCE_PLL;
  PeriphClkInitStruct.Sdmmc1ClockSelection = RCC_SDMMC1CLKSOURCE_CLK48;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief NVIC Configuration.
  * @retval None
  */
static void MX_NVIC_Init(void)
{
  /* RCC_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(RCC_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(RCC_IRQn);
  /* CAN1_TX_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(CAN1_TX_IRQn, 2, 0);
  HAL_NVIC_EnableIRQ(CAN1_TX_IRQn);
  /* CAN1_RX0_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(CAN1_RX0_IRQn, 2, 0);
  HAL_NVIC_EnableIRQ(CAN1_RX0_IRQn);
  /* CAN1_RX1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(CAN1_RX1_IRQn, 2, 0);
  HAL_NVIC_EnableIRQ(CAN1_RX1_IRQn);
  /* CAN1_SCE_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(CAN1_SCE_IRQn, 2, 0);
  HAL_NVIC_EnableIRQ(CAN1_SCE_IRQn);
  /* USART1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(USART1_IRQn, 2, 0);
  HAL_NVIC_EnableIRQ(USART1_IRQn);
  /* SDMMC1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SDMMC1_IRQn, 2, 0);
  HAL_NVIC_EnableIRQ(SDMMC1_IRQn);
  /* DMA2_Stream7_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream7_IRQn, 2, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream7_IRQn);
}

/* USER CODE BEGIN 4 */

void Warning_Handler(char* description){
	UNUSED(description);
}

static uint16_t errorInCounter = 0;

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  if (errorInCounter++ > 0){
	  return;
  }

  while(1)
  {
	  HAL_GPIO_WritePin(my_LED_DEBUG2_GPIO_Port, my_LED_DEBUG2_Pin, GPIO_PIN_RESET);
	  HAL_GPIO_WritePin(my_LED_DEBUG1_GPIO_Port, my_LED_DEBUG1_Pin, GPIO_PIN_RESET);
	  HAL_Delay(75);
	  HAL_GPIO_WritePin(my_LED_DEBUG2_GPIO_Port, my_LED_DEBUG2_Pin, GPIO_PIN_SET);
	  HAL_GPIO_WritePin(my_LED_DEBUG1_GPIO_Port, my_LED_DEBUG1_Pin, GPIO_PIN_SET);
	  HAL_Delay(75);
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
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
