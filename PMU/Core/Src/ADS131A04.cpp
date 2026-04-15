/*
 * ADS131A04.cpp
 *
 *  Created on: 2026. 3. 19.
 *      Author: yujin
 */
#include "ADS131A04.h"
#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"

ADS131A04IPBSR::ADS131A04IPBSR(SPI_HandleTypeDef* spi, GPIO_TypeDef* csP, uint16_t csN, GPIO_TypeDef* resetP, uint16_t resetN)
    : hspi(spi), csPort(csP), csPin(csN), resetPort(resetP), resetPin(resetN){}

ADS131A04IPBSR::~ADS131A04IPBSR() {}

void ADS131A04IPBSR::SpiFrameTransfer(uint32_t command) {
    uint8_t tx_cmd[24] = {0, };
    uint8_t rx_data[24] = {0, };

    // 첫 번째 워드(Status 위치)에만 명령어 4바이트 쪼개 넣기
    tx_cmd[0] = (command >> 24) & 0xFF;
    tx_cmd[1] = (command >> 16) & 0xFF;
    tx_cmd[2] = (command >> 8) & 0xFF;
    tx_cmd[3] = command & 0xFF;

    // SPI 통신 3단계 (CS Low -> 통신 -> CS High)
    HAL_GPIO_WritePin(csPort, csPin, GPIO_PIN_RESET);
    HAL_SPI_TransmitReceive(hspi, tx_cmd, rx_data, 24, 100);
    HAL_GPIO_WritePin(csPort, csPin, GPIO_PIN_SET);

    // 받아온 24바이트를 4바이트씩 6개의 방(frame_data)에 정리
    for(int i = 0; i < 6; i++)
    {
    	int idx = i * 4;
    	frame_data[i] = ((uint32_t)rx_data[idx] << 24) |
    			((uint32_t)rx_data[idx + 1] << 16) |
				((uint32_t)rx_data[idx + 2] <<  8) |
				((uint32_t)rx_data[idx + 3]);
	}
    //return frame_data;
}

bool ADS131A04IPBSR::Init()
{
 HAL_GPIO_WritePin(resetPort, resetPin, GPIO_PIN_RESET);
	HAL_Delay(5);
	HAL_GPIO_WritePin(resetPort, resetPin, GPIO_PIN_SET);
	HAL_Delay(10);

	HAL_GPIO_WritePin(ECC_ENABLE_GPIO_Port, ECC_ENABLE_Pin, GPIO_PIN_SET);

	HAL_GPIO_WritePin(SPI_WORD_SIZE_GPIO_Port, SPI_WORD_SIZE_Pin, GPIO_PIN_SET);

	HAL_GPIO_WritePin(SPI_MODE_GPIO_Port, SPI_MODE_Pin, GPIO_PIN_RESET);

	HAL_GPIO_WritePin(ADC_DRDYn_GPIO_Port, ADC_DRDYn_Pin, GPIO_PIN_SET);

	vTaskDelay(pdMS_TO_TICKS(10));

    // 0x00000000을 보내고 32비트 응답을 바로 받아옴
    SpiFrameTransfer(0x00000000);

    if((frame_data[0] & 0xFF000000) == 0xFF000000)
    {
        //uint8_t device_id = (frame_data[0] >> 16) & 0xFF;
        //printf("ADS131A04 Ready! Device ID: 0x%02X\r\n", device_id);

        Send_UNLOCK();
        return true;
    }
    else
    {
        //printf("ADS131A04 Init Failed. Rx: 0x%08lX\r\n", frame_data[0]);
        return false;
    }
}

bool ADS131A04IPBSR::Send_UNLOCK() {
    // 잠금 해제 명령어(0x06550000) 전송 (리턴값은 무시)
    SpiFrameTransfer(0x06550000); //SPI는 Byte 단위로 MSB부터 전송. ADC의 명령어 처리기는 16비트(데이터시트 p.52에 16비트 제외하고 LSB 부분 0으로 채운다고도 나와있음)
    return true;
}

uint32_t ADS131A04IPBSR::GetRawValue(uint8_t channel) {
	if(channel > 3)
	{
		return 0;
	}
    // 더미 데이터를 보내 최신 채널 값들로 배열을 갱신
    SpiFrameTransfer(0x00000000);

    // [0]은 Status, [1]은 CH0, [2]는 CH1, [3]은 CH2, [4]는 CH3, [5]는 CRC
    return frame_data[channel + 1];
}

float ADS131A04IPBSR::GetVolt(uint8_t channel) {

    int32_t raw = GetRawValue(channel);

    int32_t pure_24bit_adc = raw >> 8; //오른쪽으로 8칸 밀어서 하위 8비트(해밍 데이터)를 버림

    return (float)pure_24bit_adc * ((2.442 * 2.0f)/ 16777215.0f); // REFP 커패시터만 연결 => 칩 자체의 내부 레퍼런스 사용, 2.442V
}

void ADS131A04IPBSR::TriggerSync() {
    HAL_GPIO_WritePin(ADC_DRDYn_GPIO_Port, ADC_DRDYn_Pin, GPIO_PIN_RESET);

    for(volatile int i = 0; i < 20; i++);

    HAL_GPIO_WritePin(ADC_DRDYn_GPIO_Port, ADC_DRDYn_Pin, GPIO_PIN_SET);
}
