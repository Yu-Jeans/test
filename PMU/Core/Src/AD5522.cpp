/*
 * AD5522.cpp
 *
 *  Created on: 2026. 3. 24.
 *      Author: yujin
 */


#include "AD5522.h"
#include <cmath>
#include <cstdio>

AD5522::AD5522(
		SPI_HandleTypeDef* spi,
		GPIO_TypeDef* syncP, uint16_t syncN,
		GPIO_TypeDef* busyP, uint16_t busyN,
		GPIO_TypeDef* loadP, uint16_t loadN,
		GPIO_TypeDef* resetP, uint16_t resetN)
    : hspi(spi), syncPort(syncP), syncPin(syncN), busyPort(busyP), busyPin(busyN), loadPort(loadP), loadPin(loadN), resetPort(resetP), resetPin(resetN){}

// =========================================================================
// 1. 초기화 및 시스템 제어 (System Control)
// =========================================================================
bool AD5522::Init() {
    // 1. 하드웨어 리셋 시작 (Active-Low)
    HAL_GPIO_WritePin(resetPort, resetPin, GPIO_PIN_RESET);
    // 2. 최소 4us 이상 유지 (t16a). 넉넉하게 1ms 대기
    HAL_Delay(1);
    // 3. 리셋 신호 해제 (High)
    HAL_GPIO_WritePin(resetPort, resetPin, GPIO_PIN_SET);

    // 4. 칩이 초기화를 끝내고 BUSY 핀을 High로 올려줄 때까지 대기
    uint32_t start_time = HAL_GetTick();
    while (HAL_GPIO_ReadPin(busyPort, busyPin) == GPIO_PIN_RESET) {
        if ((HAL_GetTick() - start_time) > 10) {
            return false; // 10ms가 지나도 안 깨어나면 회로 단락 등 하드웨어 에러
        }
    }
    SetSystemDefault(0x0F, 0x0F);
    return true;
}

// System Control Register 기본 세팅
void AD5522::SetSystemDefault(uint8_t cpolh_mask, uint8_t cl_mask) {
    uint32_t reg = 0;

    // 주소 비트 (B27 ~ B22) : 000000 (System Control Register 선택)
    // 기능 전원
    reg |= (1 << 13); // CPBIASEN: 내부 비교기 메인 전원 ON
    reg |= (1 << 8);  // Guard EN: 가드 앰프 메인 전원 ON
    reg |= (1 << 7);  // GAIN1=1, GAIN0=0
    reg |= (1 << 5);  // TMP ENABLE: 칩 과열(기본 130도) 시 자동 셧다운 기능 ON
    // 알람 핀 세팅
    reg |= (1 << 10); // CLAMP ALM: 클램프 작동 시 CGALM 핀으로 알람 출력
    reg |= (1 << 11); // GUARD ALM: 가드 에러 시 CGALM 핀으로 알람 출력

    reg |= (uint32_t)(cpolh_mask & 0x0F) << 18; // CPOLH 3~0
	reg |= (uint32_t)(cl_mask & 0x0F) << 14;    // CL 3~0

	Write29Bits(reg);
}

// =========================================================================
// 2. 채널 및 스위치 경로 설정 (PMU Register)
// =========================================================================
void AD5522::SetChannelMode(uint8_t ch_mask, bool enable, ForceMode f_mode, CurrentRange range, MeasureMode m_mode) {
    uint32_t reg = 0;

    // 1. 채널 선택 (B27 ~ B24)
    reg |= (ch_mask & 0x0F) << 24;

    // 2. 모드 선택 (B23 ~ B22): PMU Register는 00이므로 생략
    // reg |= (0b00 << 22);

    // 3. 채널 출력 활성화 (B21)
    if (enable) {
        reg |= (1 << 21);
    }

    // 4. Force 모드 설정 (B20 ~ B19)
    reg |= (f_mode & 0b11) << 19;

    // 5. 전류 범위 설정 (B17 ~ B15)
    reg |= (range & 0b111) << 15;

    // 6. Measure 모드 설정 (B14 ~ B13)
    reg |= (m_mode & 0b11) << 13;

    // 7. 기타 필수 스위치 ON
    reg |= (1 << 12); // FIN : DAC 신호를 내부 Amp에 연결 (필수)
    reg |= (1 << 9);  // CL : 이 채널의 클램프 보호 기능 활성화 (안전 필수)
    reg |= (1 << 8);

    // 칩으로 전송
    Write29Bits(reg);
}

// =========================================================================
// 3. 아날로그 레벨 장전 및 출력 (DAC Register & LOAD Toggle)
// =========================================================================
// range_bits == 0b101로 둔 이유? current range 나타내는 게 0b000 ~ 0b100. 따라서 전압은 0b101이라고 정의하진 않았지만 정의
void AD5522::SetForceValue(Channel ch, uint8_t range_bits, float value) {
    uint16_t code = (range_bits == 0b101) ? CalculateVoltageToDAC(value) : CalculateCurrentToDAC(range_bits, value);
    WriteDAC(ch, DAC_FIN, range_bits, code);
}

void AD5522::SetClamp(Channel ch, DAC_Type type, uint8_t range_bits, float value) {
    uint16_t code = (range_bits == 0b101) ? CalculateVoltageToDAC(value) : CalculateCurrentToDAC(range_bits, value);
    WriteDAC(ch, type, range_bits, code);
}

void AD5522::SetComparator(Channel ch, DAC_Type type, uint8_t range_bits, float value) {
    uint16_t code = (range_bits == 0b101) ? CalculateVoltageToDAC(value) : CalculateCurrentToDAC(range_bits, value);
    WriteDAC(ch, type, range_bits, code);
}


// =========================================================================
// 4. Private 하위 유틸리티 함수들 (통신 및 수학 연산)
// =========================================================================
void AD5522::WriteDAC(Channel ch, DAC_Type type, uint8_t range_bits, uint16_t dac_code) {
    uint32_t reg = 0;

    // B27~B24: 채널 선택
    reg |= (ch & 0x0F) << 24;

    // B23~B22: 모드 선택 (DAC Register는 11)
    reg |= (0b11) << 22;

    // B21~B19 (A5~A3): 부품 선택 (메인 출력인 FIN DAC 선택)
    reg |= (type & 0b111) << 19;

    // B18~B16 (A2~A0): Range 선택 (Voltage Range는 101)
    reg |= (range_bits) << 16;

    // B15~B0 (D15~D0): 16비트로 변환된 실제 데이터 값
    reg |= dac_code;

    // 3. 조립 완료! SPI로 전송
    Write29Bits(reg);
}

// 29-bit SPI 전송 및 BUSY 대기 함수 (Private)
void AD5522::Write29Bits(uint32_t data) {
    // STM32는 8비트 단위로 통신하므로, 29비트를 보내기 위해 32비트(4바이트) 배열을 준비
    uint8_t spi_tx[4];
    spi_tx[0] = (data >> 24) & 0xFF;
    spi_tx[1] = (data >> 16) & 0xFF;
    spi_tx[2] = (data >> 8)  & 0xFF;
    spi_tx[3] = data & 0xFF;

    // 1. SYNC Low (통신 시작)
    HAL_GPIO_WritePin(syncPort, syncPin, GPIO_PIN_RESET);
    // 2. 데이터 4바이트(32클럭) 전송
    HAL_SPI_Transmit(hspi, spi_tx, 4, 10);
    // 3. SYNC High (통신 종료)
    HAL_GPIO_WritePin(syncPort, syncPin, GPIO_PIN_SET);

    uint32_t start_time = HAL_GetTick();
	while (HAL_GPIO_ReadPin(busyPort, busyPin) == GPIO_PIN_RESET) {
		if ((HAL_GetTick() - start_time) > 10) { // 10ms 타임아웃
			printf("AD5522: BUSY pin timeout!\r\n");
			break;
		}
	}
}

uint16_t AD5522::CalculateVoltageToDAC(float target_v) {
    float result = ((target_v + 11.25f) * 65536.0f) / 22.5f; // VREF=5V, Offset=0x8000 기준 단순화

    if (result > 65535.0f){
    	result = 65535.0f;
    }

    if (result < 0.0f){
    	result = 0.0f;
    }

    return (uint16_t)result;
}

uint16_t AD5522::CalculateCurrentToDAC(uint8_t range_bits, float target_i) {
    float rsense = GetRsenseValue(range_bits);

    float target_i_A = target_i / 1000000.0f;

    float result = ((target_i_A * rsense * 10.0f * 65536.0f) / 22.5f) + 32768.0f;

    if (result > 65535.0f){
    	result = 65535.0f;
    }
    if (result < 0.0f){
    	result = 0.0f;
    }

    return (uint16_t)result;
}

float AD5522::GetRsenseValue(uint8_t range_bits) {
    switch(range_bits) {
        case RANGE_5uA:
        	return 200000.0f;
        case RANGE_20uA:
        	return 50000.0f;
        case RANGE_200uA:
        	return 5000.0f;
        case RANGE_2mA:
        	return 500.0f;
        default:
        	return 500.0f;
    }
}
