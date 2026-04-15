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
#include "PMU.h"
#include "cmsis_os.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "usb_device.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "Logger.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
extern "C" {
    void StartDefaultTask(void *argument);
    void MX_FREERTOS_Init(void);
}

extern PMU mySystem;


struct CommandDef {
    const char* cmd_str;
    uint8_t cmd_type;
    int required_args;
};

static const CommandDef cmdDict[] = {
    {"stop",     CMD_EMERGENCY_STOP,   0},
    {"save_cal", CMD_SAVE_CALIBRATION, 0},
    {"fv",       CMD_FORCE_VOLTAGE,    2},
    {"fi",       CMD_FORCE_CURRENT,    2},
    {"hzv",      CMD_HIGH_Z_V,         1},
    {"hzi",      CMD_HIGH_Z_I,         1},
    {"mv",       CMD_MEAS_VOLTAGE,     1},
    {"mi",       CMD_MEAS_CURRENT,     1},
    {"mt",       CMD_MEAS_TEMP,        1},
    {"tune",     CMD_TUNE_DYNAMIC,     1},
    {"cff",      CMD_SET_CFF,          2},
    {"comp",     CMD_SET_CCOMP,        2},
    {"set_vcal", 0xFF,                 3},
    {"set_ical", 0xFF,                 3},
    {"set_prot", 0xFF,                 2}
};

static const int NUM_CMDS = sizeof(cmdDict) / sizeof(CommandDef);


#define CH0_CPOH_NORMAL  GPIO_PIN_SET
#define CH0_CPOL_NORMAL  GPIO_PIN_SET

#define CH1_CPOH_NORMAL  GPIO_PIN_SET
#define CH1_CPOL_NORMAL  GPIO_PIN_SET

#define CH2_CPOH_NORMAL  GPIO_PIN_SET
#define CH2_CPOL_NORMAL  GPIO_PIN_SET

#define CH3_CPOH_NORMAL  GPIO_PIN_SET
#define CH3_CPOL_NORMAL  GPIO_PIN_SET

typedef struct {
    GPIO_TypeDef* port;
    uint16_t pin;
    uint8_t normal;
} ClampPin_t;

static const ClampPin_t Comparator_PINS[8] = {
    {CPOH0_GPIO_Port, CPOH0_Pin, CH0_CPOH_NORMAL}, {CPOL0_GPIO_Port, CPOL0_Pin, CH0_CPOL_NORMAL},
    {CPOH1_GPIO_Port, CPOH1_Pin, CH1_CPOH_NORMAL}, {CPOL1_GPIO_Port, CPOL1_Pin, CH1_CPOL_NORMAL},
    {CPOH2_GPIO_Port, CPOH2_Pin, CH2_CPOH_NORMAL}, {CPOL2_GPIO_Port, CPOL2_Pin, CH2_CPOL_NORMAL},
    {CPOH3_GPIO_Port, CPOH3_Pin, CH3_CPOH_NORMAL}, {CPOL3_GPIO_Port, CPOL3_Pin, CH3_CPOL_NORMAL}
};
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for PMU_Task */
osThreadId_t PMU_TaskHandle;
const osThreadAttr_t PMU_Task_attributes = {
  .name = "PMU_Task",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for commandTask */
osThreadId_t commandTaskHandle;
const osThreadAttr_t commandTask_attributes = {
  .name = "commandTask",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for uartLoggingTask */
osThreadId_t uartLoggingTaskHandle;
const osThreadAttr_t uartLoggingTask_attributes = {
  .name = "uartLoggingTask",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for ADC_Task */
osThreadId_t ADC_TaskHandle;
const osThreadAttr_t ADC_Task_attributes = {
  .name = "ADC_Task",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityAboveNormal,
};
/* Definitions for uartQueue */
osMessageQueueId_t uartQueueHandle;
const osMessageQueueAttr_t uartQueue_attributes = {
  .name = "uartQueue"
};
/* Definitions for usbRxQueue */
osMessageQueueId_t usbRxQueueHandle;
const osMessageQueueAttr_t usbRxQueue_attributes = {
  .name = "usbRxQueue"
};
/* Definitions for pmuCmdQueue */
osMessageQueueId_t pmuCmdQueueHandle;
const osMessageQueueAttr_t pmuCmdQueue_attributes = {
  .name = "pmuCmdQueue"
};


/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
/* USER CODE END FunctionPrototypes */

void StartPMU_Task(void *argument);
void StartCommandTask(void *argument);
void StartUartLoggingTask(void *argument);
void StartADC_Task(void *argument);

extern void MX_USB_DEVICE_Init(void);
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
  /* creation of uartQueue */
  uartQueueHandle = osMessageQueueNew (4, sizeof(PMU_Data_t), &uartQueue_attributes);

  /* creation of usbRxQueue */
  usbRxQueueHandle = osMessageQueueNew (128, sizeof(uint8_t), &usbRxQueue_attributes);

  /* creation of pmuCmdQueue */
  pmuCmdQueueHandle = osMessageQueueNew (8, sizeof(PMU_CmdPacket_t), &pmuCmdQueue_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of PMU_Task */
  PMU_TaskHandle = osThreadNew(StartPMU_Task, NULL, &PMU_Task_attributes);

  /* creation of commandTask */
  commandTaskHandle = osThreadNew(StartCommandTask, NULL, &commandTask_attributes);

  /* creation of uartLoggingTask */
  uartLoggingTaskHandle = osThreadNew(StartUartLoggingTask, NULL, &uartLoggingTask_attributes);

  /* creation of ADC_Task */
  ADC_TaskHandle = osThreadNew(StartADC_Task, NULL, &ADC_Task_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartPMU_Task */
/**
  * @brief  Function implementing the PMU_Task thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartPMU_Task */
void StartPMU_Task(void *argument)
{
  /* USER CODE BEGIN StartPMU_Task */
  mySystem.Init();

  PMU_CmdPacket_t order;
  /* Infinite loop */
  for(;;)
  {
	    if (osMessageQueueGet(pmuCmdQueueHandle, &order, NULL, osWaitForever) == osOK)
		{
			switch (order.cmd_type)
			{
				case CMD_FORCE_VOLTAGE:
					mySystem.SetOutputVoltage(order.channel, order.value);
					break;

				case CMD_FORCE_CURRENT:
					mySystem.SetOutputCurrent(order.channel, order.value);
					break;

				case CMD_HIGH_Z_V:
				    mySystem.SetHighZ(order.channel, AD5522::HIGH_Z_V);
				    break;

				case CMD_HIGH_Z_I:
				    mySystem.SetHighZ(order.channel, AD5522::HIGH_Z_I);
				    break;

				case CMD_MEAS_VOLTAGE:
					mySystem.MeasureVolt(order.channel);
					osDelay(pdMS_TO_TICKS(5));
					xTaskNotifyGive((TaskHandle_t)commandTaskHandle);
					break;

				case CMD_MEAS_CURRENT:
					mySystem.MeasureCurrent(order.channel);
					osDelay(pdMS_TO_TICKS(5));
					xTaskNotifyGive((TaskHandle_t)commandTaskHandle);
					break;

				case CMD_MEAS_TEMP:
				    mySystem.MeasureTemp(order.channel);
				    osDelay(pdMS_TO_TICKS(5));
				    xTaskNotifyGive((TaskHandle_t)commandTaskHandle);
				    break;

				case CMD_EMERGENCY_STOP:
					mySystem.Emergency_Stop();
					break;

				case CMD_SAVE_CALIBRATION:
				    mySystem.SaveCalibrationToEEPROM();
				    break;

				case CMD_TUNE_DYNAMIC:
					mySystem.TuneDynamicOffset(order.channel);
					osDelay(pdMS_TO_TICKS(5));
					xTaskNotifyGive((TaskHandle_t)commandTaskHandle);
					break;

				case CMD_SET_CFF:
				    mySystem.SetCFF(order.channel, order.mux_val);
				    break;

				case CMD_SET_CCOMP:
				    mySystem.SetCCOMP(order.channel, order.mux_val);
				    break;
			}
			// 명령을 수행할 때마다 UART 로깅창(uartQueue)으로 최신 상태 업데이트
			osMessageQueuePut(uartQueueHandle, &mySystem.latestData, 0, 0);
		}
  }
  /* USER CODE END StartPMU_Task */
}

/* USER CODE BEGIN Header_StartUartLoggingTask */
/**
* @brief Function implementing the uartLoggingTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartUartLoggingTask */
void StartUartLoggingTask(void *argument)
{
  /* USER CODE BEGIN StartUartLoggingTask */
  extern UART_HandleTypeDef huart6; // main.cpp에 있는 UART6 핸들 가져오기
  PMU_Data_t logData;
  char msg[256];
  /* Infinite loop */
  for(;;)
  {
	    if (osMessageQueueGet(uartQueueHandle, &logData, NULL, osWaitForever) == osOK) {
			int len = 0;

			// TEMP 알람 발생 시
			if (logData.system_status == SYS_ALARM_TEMP) {
				len = sprintf(msg, "\r\n[CRITICAL] TEMPERATURE ALARM!!! SYSTEM STOPPED.\r\n");
			}
			// CGALM 알람 발생 시
			else if (logData.system_status == SYS_ALARM_CGALM) {
				len = sprintf(msg, "\r\n[CRITICAL] CLAMP(CG) ALARM!!! SYSTEM STOPPED.\r\n");
			}
			// 비상시, comparator 상한, 하한 감지되었을 때 (ADC_Task가 보냄)
			else if (logData.comparator_status != 0) {
				len = sprintf(msg, "\r\n[WARN] COMPARATOR OUT OF BOUNDS! (STS:0x%02X)\r\n"
								   "  => [Volt] CH0:%.3f, CH1:%.3f, CH2:%.3f, CH3:%.3f\r\n"
								   "  => [Curr] CH0:%.3f, CH1:%.3f, CH2:%.3f, CH3:%.3f\r\n",
							  logData.comparator_status,
							  logData.voltage[0], logData.voltage[1],
							  logData.voltage[2], logData.voltage[3],
			                  logData.current[0], logData.current[1],
			                  logData.current[2], logData.current[3]);
			}
			// 평상시, 정상 측정 데이터일 때 (PMU_Task가 보냄)
			else {
				len = sprintf(msg, "[LOG] V0:%.3f, V1:%.3f, V2:%.3f, V3:%.3f, I0:%.3f, I1:%.3f, I2:%.3f, I3:%.3f (STS:OK)\r\n",
							  logData.voltage[0], logData.voltage[1],
							  logData.voltage[2], logData.voltage[3],
							  logData.current[0], logData.current[1],
							  logData.current[2], logData.current[3]);
			}
			HAL_UART_Transmit(&huart6, (uint8_t*)msg, len, 100);
		}
  }
  /* USER CODE END StartUartLoggingTask */
}


void ProcessCommand(const char* cmdBuffer) {
    char cmdStr[16] = {0};
    int ch = -1;
    float v1 = 0.0f, v2 = 0.0f;

    int parsed = sscanf(cmdBuffer, "%15s %d %f %f", cmdStr, &ch, &v1, &v2);

    if (parsed < 1){
    	return;
    }

    for(int i = 0; cmdStr[i]; i++) cmdStr[i] = tolower(cmdStr[i]);

    const CommandDef* matchedCmd = NULL;
    for (int i = 0; i < NUM_CMDS; i++) {
        if (strcmp(cmdStr, cmdDict[i].cmd_str) == 0) {
            matchedCmd = &cmdDict[i];
            break;
        }
    }

    if (!matchedCmd) {
        LOG_E("Unknown Command: %s\r\n", cmdStr);
        return;
    }

    if (parsed < matchedCmd->required_args + 1) {
        LOG_E("'%s' requires %d arguments.\r\n", matchedCmd->cmd_str, matchedCmd->required_args);
        return;
    }

    if (matchedCmd->required_args > 0 && (ch < 0 || ch > 3)) {
        LOG_E("Invalid Channel (0~3)\r\n");
        return;
    }

    if (strcmp(cmdStr, "set_vcal") == 0) {
        mySystem.SetStaticCalV(ch, v1, v2);
        LOG_I("CH%d Static V_CAL Updated (Offset:%.3f, Gain:%.3f)\r\n", ch, v1, v2);
        return;
    }
    if (strcmp(cmdStr, "set_ical") == 0) {
        mySystem.SetStaticCalI(ch, v1, v2);
        LOG_I("CH%d Static I_CAL Updated (Offset:%.3f, Gain:%.3f)\r\n", ch, v1, v2);
        return;
    }
    if (strcmp(cmdStr, "set_prot") == 0) {
		char bin1[9] = {0}; // CPOLH 문자열 담을 곳
		char bin2[9] = {0}; // CL 문자열 담을 곳

		if (sscanf(cmdBuffer, "%*s %8s %8s", bin1, bin2) == 2) {

			int cpolh = strtol(bin1, NULL, 2);
			int cl = strtol(bin2, NULL, 2);

			mySystem.SetProtection(cpolh, cl);

			LOG_I("System Protection Updated (CPOLH: %04ld, CL: %04ld)\r\n",
				  strtol(bin1, NULL, 10), strtol(bin2, NULL, 10));
		} else {
			LOG_E("Format: set_prot <cpolh_bits> <cl_bits> (ex: set_prot 1111 1111)\r\n");
		}
		return;
	}

    PMU_CmdPacket_t order;
    order.cmd_type = (PMU_CmdType_t)matchedCmd->cmd_type;
    order.channel = ch;
    order.value = v1;
    order.mux_val = (int)v1;

    osMessageQueuePut(pmuCmdQueueHandle, &order, 0, portMAX_DELAY);

    if (order.cmd_type == CMD_EMERGENCY_STOP) {
        LOG_W("EMERGENCY STOP!\r\n");
    }
    else if (order.cmd_type == CMD_SAVE_CALIBRATION) {
        LOG_I("Calibration Data Saved to EEPROM Successfully!\r\n");
    }

    else if (order.cmd_type == CMD_MEAS_VOLTAGE || order.cmd_type == CMD_MEAS_CURRENT ||
             order.cmd_type == CMD_MEAS_TEMP || order.cmd_type == CMD_TUNE_DYNAMIC)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        if (order.cmd_type == CMD_MEAS_VOLTAGE) {
			LOG_I("CH%d MV: %.4f V\r\n", ch, mySystem.latestData.voltage[ch]);
		}
		else if (order.cmd_type == CMD_MEAS_CURRENT) {
			LOG_I("CH%d MI: %.3f uA\r\n", ch, mySystem.latestData.current[ch]);
		}
		else if (order.cmd_type == CMD_MEAS_TEMP) {
			LOG_I("CH%d MT: %.1f C\r\n", ch, mySystem.latestData.temp[ch]);
		}
        else if (order.cmd_type == CMD_TUNE_DYNAMIC) {
            if (mySystem.current_force_mode[ch] == AD5522::FV_MODE){
            	LOG_I("CH%d Tuned! New MV: %.4f V\r\n", ch, mySystem.latestData.voltage[ch]);
            }
            else{
            	LOG_I("CH%d Tuned! New MI: %.3f uA\r\n", ch, mySystem.latestData.current[ch]);
            }
        }
    }
    else {
    	if (order.cmd_type == CMD_FORCE_VOLTAGE){
    		LOG_I("CH%d FV: %.3f V\r\n", ch, v1);
    	}
		else if (order.cmd_type == CMD_FORCE_CURRENT){
			LOG_I("CH%d FI: %.1f uA\r\n", ch, v1);
		}
		else if (order.cmd_type == CMD_HIGH_Z_V){
			LOG_I("CH%d HZV: OK\r\n", ch);
		}
		else if (order.cmd_type == CMD_HIGH_Z_I){
			LOG_I("CH%d HZI: OK\r\n", ch);
		}
		else if (order.cmd_type == CMD_SET_CFF){
			LOG_I("CH%d CFF: MUX_%d\r\n", ch, (int)v1);
		}
		else if (order.cmd_type == CMD_SET_CCOMP){
			LOG_I("CH%d COMP: MUX_%d\r\n", ch, (int)v1);
		}
	}
}

void StartCommandTask(void *argument)
{
    /* init code for USB_DEVICE */
    MX_USB_DEVICE_Init();

    char rxBuffer[64];
    uint8_t index = 0;
    uint8_t rxData;

    LOG_I("\r\n=== PMU USB Command Mode ===\r\n");

    printf("PMU> ");

    for(;;)
    {
        if (osMessageQueueGet(usbRxQueueHandle, &rxData, NULL, portMAX_DELAY) == osOK)
        {
            printf("%c", rxData); // 사용자가 친 글자 화면에 보여주기

            if (rxData == '\r' || rxData == '\n') // 엔터키를 쳤을 때
            {
                if (index > 0) {
                    rxBuffer[index] = '\0'; // 문자열 닫기

                    printf("\r\n");

                    ProcessCommand(rxBuffer);

                    index = 0; // 버퍼 비우기

                    printf("PMU> "); // 다음 입력 대기
                }
            }
            else if (rxData == '\b' || rxData == 0x7F) { // 백스페이스 처리
                if (index > 0) index--;
            }
            else { // 일반 글자 모으기
                if (index < 63) rxBuffer[index++] = rxData;
            }
        }
    }
}

/* USER CODE BEGIN Header_StartADC_Task */
/**
* @brief Function implementing the ADC_Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartADC_Task */
void StartADC_Task(void *argument)
{
  /* USER CODE BEGIN StartADC_Task */
  /* Infinite loop */
  for(;;)
  {
	  // HAL_GPIO_EXTI_Callback에서 보내는 알람(인터럽트) 대기
	  ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

	  if (mySystem.latestData.system_status == SYS_ALARM_TEMP ||
		  mySystem.latestData.system_status == SYS_ALARM_CGALM) {
		  continue;
	  }

	  uint8_t comparator_mask = 0;
	  for (int i = 0; i < 8; i++) {
		  if (HAL_GPIO_ReadPin(Comparator_PINS[i].port, Comparator_PINS[i].pin) != Comparator_PINS[i].normal) {
			  comparator_mask |= (1 << i);
		  }
	  }
	  mySystem.latestData.comparator_status = comparator_mask;

	  if (comparator_mask != 0) {
		  mySystem.Emergency_Stop();
		  mySystem.EmergencyMeasureAll();
		  osMessageQueuePut(uartQueueHandle, &mySystem.latestData, 0, 0);
		  continue;
	  }

	  mySystem.UpdateMeasurementsFromADC();
  }
  /* USER CODE END StartADC_Task */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */
