/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
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
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>

#include "iwdg.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
extern uint8_t rx_buff[];
extern uint8_t rx_data_len;
extern UART_HandleTypeDef huart1;
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
static uint8_t tx_buf[128];
/* USER CODE END Variables */
/* Definitions for controlTask */
osThreadId_t controlTaskHandle;
const osThreadAttr_t controlTask_attributes = {
  .name = "controlTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityHigh,
};
/* Definitions for cmdTask */
osThreadId_t cmdTaskHandle;
const osThreadAttr_t cmdTask_attributes = {
  .name = "cmdTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for txTask */
osThreadId_t txTaskHandle;
const osThreadAttr_t txTask_attributes = {
  .name = "txTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for statusTask */
osThreadId_t statusTaskHandle;
const osThreadAttr_t statusTask_attributes = {
  .name = "statusTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for rxQueue */
osMessageQueueId_t rxQueueHandle;
const osMessageQueueAttr_t rxQueue_attributes = {
  .name = "rxQueue"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartControlTask(void *argument);
void StartCmdTask(void *argument);
void StartTxTask(void *argument);
void StartStatusTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* creation of rxQueue */
  rxQueueHandle = osMessageQueueNew (10, 64, &rxQueue_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of controlTask */
  controlTaskHandle = osThreadNew(StartControlTask, NULL, &controlTask_attributes);

  /* creation of cmdTask */
  cmdTaskHandle = osThreadNew(StartCmdTask, NULL, &cmdTask_attributes);

  /* creation of txTask */
  txTaskHandle = osThreadNew(StartTxTask, NULL, &txTask_attributes);

  /* creation of statusTask */
  statusTaskHandle = osThreadNew(StartStatusTask, NULL, &statusTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartControlTask */
/**
  * @brief  Function implementing the controlTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartControlTask */
void StartControlTask(void *argument)
{
  /* USER CODE BEGIN StartControlTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
    HAL_GPIO_TogglePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin);
  }
  /* USER CODE END StartControlTask */
}

/* USER CODE BEGIN Header_StartCmdTask */
/**
* @brief Function implementing the cmdTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartCmdTask */
void StartCmdTask(void *argument)
{
  /* USER CODE BEGIN StartCmdTask */
  uint8_t rx_buf[64];

  static uint8_t line_buf[128];
  static uint16_t line_idx = 0;
  /* Infinite loop */
  for(;;)
  {
	  if(osMessageQueueGet(rxQueueHandle, rx_buf, NULL, osWaitForever) == osOK){
		  int i = 0;
		  while(rx_buf[i] != '\0' && i < 64){
			  line_buf[line_idx] = rx_buf[i];
			  ++line_idx;

			  if(line_idx >= sizeof(line_buf) - 1){
				  line_idx = 0;
			  }

			  if(rx_buf[i] == '\n'){
				  line_buf[line_idx] = '\0';
				  printf("Data: %s", line_buf);

//				  uint16_t data_len = strlen((char *) line_buf);
//				  memcpy(tx_buf, "ACK: ", 5);
//				  memcpy(&tx_buf[5], line_buf, data_len);
//				  uint16_t tx_len_sum = 5 + data_len;
//
//				  while(huart1.gState == HAL_UART_STATE_BUSY_TX);
//
//				  HAL_UART_Transmit_DMA(&huart1, tx_buf, tx_len_sum);

				  line_idx = 0;
			  }
			  ++i;
		  }
	  }
  }
  /* USER CODE END StartCmdTask */
}

/* USER CODE BEGIN Header_StartTxTask */
/**
* @brief Function implementing the txTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTxTask */
void StartTxTask(void *argument)
{
  /* USER CODE BEGIN StartTxTask */
  uint32_t tx_c = 0;
  /* Infinite loop */
  for(;;)
  {
    int data_len = snprintf((char *) tx_buf, sizeof(tx_buf), "ACK %lu\n", 500 + tx_c);
   	HAL_UART_Transmit_DMA(&huart1, tx_buf, data_len);
   	++tx_c;
    osDelay(500);
  }
  /* USER CODE END StartTxTask */
}

/* USER CODE BEGIN Header_StartStatusTask */
/**
* @brief Function implementing the statusTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartStatusTask */
void StartStatusTask(void *argument)
{
  /* USER CODE BEGIN StartStatusTask */
  /* Infinite loop */
  for(;;)
  {
    HAL_IWDG_Refresh(&hiwdg);
    osDelay(150);
  }
  /* USER CODE END StartStatusTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

