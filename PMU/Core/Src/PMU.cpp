/*
 * PMU.cpp
 *
 *  Created on: 2026. 3. 24.
 *      Author: yujin
 */

#include "PMU.h"
#include "cmsis_os2.h"
#include "FreeRTOS.h"
#include "task.h"
#include "Logger.h"

PMU::PMU(SPI_HandleTypeDef* hspi_adc,
		GPIO_TypeDef* csPort_adc, uint16_t csPin_adc,
		GPIO_TypeDef* resetPort_adc, uint16_t resetPin_adc,

		SPI_HandleTypeDef* hspi_pmu,
		GPIO_TypeDef* syncP_pmu, uint16_t syncN_pmu,
		GPIO_TypeDef* busyP_pmu, uint16_t busyN_pmu,
		GPIO_TypeDef* loadP_pmu, uint16_t loadN_pmu,
		GPIO_TypeDef* resetP_pmu, uint16_t resetN_pmu,

         I2C_HandleTypeDef* hi2c_eeprom, uint16_t addr_eeprom)

    : ADC_IC(hspi_adc, csPort_adc, csPin_adc, resetPort_adc, resetPin_adc),
	  PMU_IC(hspi_pmu, syncP_pmu, syncN_pmu, busyP_pmu, busyN_pmu, loadP_pmu, loadN_pmu, resetP_pmu, resetN_pmu),
	  myEEPROM(hi2c_eeprom, addr_eeprom){}

PMU::~PMU() {
}

bool PMU::Init() {
    bool success = true;

    HAL_GPIO_WritePin(GPIOE, GPIO_PIN_13, GPIO_PIN_RESET); // ADC 리셋 (PE13) Low
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_14, GPIO_PIN_RESET); // PMU 리셋 (PE14) Low
	HAL_Delay(5);                                          // 5ms 대기 (숨죽임)
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_13, GPIO_PIN_SET);   // ADC 리셋 해제 (High)
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_14, GPIO_PIN_SET);   // PMU 리셋 해제 (High)
	HAL_Delay(10);                                         // 칩 내부가 완전히 켜질 때까지 10ms 대기

    // EEPROM 초기화 (myEEPROM)
    if (myEEPROM.Init()) {
		if (myEEPROM.LoadCalibration(&myCalData)) {
			LOG_I("PMU: EEPROM Calibration Data Loaded!\r\n");

			if (myCalData.v_gain[0] < 0.5f || myCalData.v_gain[0] > 2.0f) {
				LOG_W("PMU: EEPROM is Empty. Setting Default!\r\n");

				for(int i=0; i<4; i++) {
					myCalData.v_offset[i] = 0.0f; // 오차 없음
					myCalData.v_gain[i] = 1.0f;   // 비율 1:1
					myCalData.i_offset[i] = 0.0f;
					myCalData.i_gain[i] = 1.0f;
				}
			}
		}
    }
	else {
		// EEPROM 칩 자체가 응답하지 않는 경우
		LOG_E("PMU: EEPROM Hardware Init Failed!\r\n");
		success = false;
    }

    // ADC 초기화 (ADC_IC)
    if (!ADC_IC.Init()) {
    	LOG_E("PMU: [FATAL] ADC Hardware Init Failed!\r\n");
        success = false;
    }

    // AD5522 초기화 (PMU_IC)
    if (!PMU_IC.Init()) {
    	LOG_E("PMU: [FATAL] AD5522 Hardware Init Failed!\r\n");
        success = false;
    }
	// PMU 안전 한계선(Clamp, Comparator) 및 초기값 설정
	for (int ch = 0; ch < 4; ch++) {
		// 💡 [핵심 수정] 비트 마스크(1, 2, 4, 8)로 정확하게 채널 선택!
		AD5522::Channel current_ch = (AD5522::Channel)(1 << ch);

		PMU_IC.SetClamp(current_ch, AD5522::DAC_CLH, 0b101, 5.5f);
		PMU_IC.SetClamp(current_ch, AD5522::DAC_CLL, 0b101, -5.5f);

		PMU_IC.SetComparator(current_ch, AD5522::DAC_CPH, 0b101, 5.0f);
		PMU_IC.SetComparator(current_ch, AD5522::DAC_CPL, 0b101, -5.0f);

		PMU_IC.SetForceValue(current_ch, 0b101, 0.0f);
	}

	// PMU 실제 출력 스위치 ON
	for (int ch = 0; ch < 4; ch++) {
		AD5522::Channel current_ch = (AD5522::Channel)(1 << ch);
		PMU_IC.SetChannelMode(current_ch, true, AD5522::FV_MODE, AD5522::RANGE_2mA, AD5522::MI_MODE);

		current_state_range[ch]  = AD5522::RANGE_2mA;
        current_force_mode[ch]   = AD5522::FV_MODE;
        current_measure_mode[ch] = AD5522::MI_MODE;
	}

    // 칩들이 깨어난 후 PMU 차원의 추가 설정
    if (success) {
    	LOG_I("PMU: Initialization Complete. System Online.\r\n");
	} else {
		LOG_E("PMU: Initialization Failed. System Halted.\r\n");
	}
    return success;
}



float PMU::GetRangeResistance(AD5522::CurrentRange range) {
    switch(range) {
        case AD5522::RANGE_5uA:   return 200000.0f; // 200 kΩ
        case AD5522::RANGE_20uA:  return 50000.0f;  // 50 kΩ
        case AD5522::RANGE_200uA: return 5000.0f;   // 5 kΩ
        case AD5522::RANGE_2mA:   return 500.0f;    // 500 Ω
        default:                  return 1.0f;
    }
}


void PMU::SetOutputVoltage(int ch, float target_volt) {
    AD5522::Channel dac_ch = (AD5522::Channel)(1 << ch);

    target_v[ch] = target_volt;

	float calibrated_volt = (target_volt - myCalData.v_offset[ch] - dynamic_v_offset[ch]) / myCalData.v_gain[ch];

	PMU_IC.SetChannelMode(dac_ch, true, AD5522::FV_MODE, AD5522::RANGE_2mA, current_measure_mode[ch]);
    PMU_IC.SetForceValue(dac_ch, 0b101, calibrated_volt);

    current_force_mode[ch] = AD5522::FV_MODE;
}



void PMU::SetOutputCurrent(int ch, float target_current_uA) {
    AD5522::Channel dac_ch = (AD5522::Channel)(1 << ch);

    target_i[ch] = target_current_uA;

	AD5522::CurrentRange auto_range;

	float abs_current = (target_current_uA < 0) ? -target_current_uA : target_current_uA;

	if (abs_current <= 5.0f) {
		auto_range = AD5522::RANGE_5uA;
	} else if (abs_current <= 20.0f) {
		auto_range = AD5522::RANGE_20uA;
	} else if (abs_current <= 200.0f) {
		auto_range = AD5522::RANGE_200uA;
	} else if (abs_current <= 2000.0f) {
		auto_range = AD5522::RANGE_2mA;
	} else {
		LOG_E("\r\n[ERROR] CH%d Current out of limit (Max 2mA)\r\n", ch);
		return;
	}

    float calibrated_current = (target_current_uA - myCalData.i_offset[ch] - dynamic_i_offset[ch]) / myCalData.i_gain[ch];

    PMU_IC.SetChannelMode(dac_ch, true, AD5522::FI_MODE, auto_range, current_measure_mode[ch]);
    PMU_IC.SetForceValue(dac_ch, auto_range, calibrated_current);

    current_state_range[ch] = auto_range;
    current_force_mode[ch] = AD5522::FI_MODE;
}

void PMU::SetHighZ(int ch, AD5522::ForceMode hz_mode) {
    AD5522::Channel dac_ch = (AD5522::Channel)(1 << ch);

    // 채널은 Enable(true) 상태로 두고 Force 모드만 High-Z(0b10)로 변경하여 출력을 차단
    PMU_IC.SetChannelMode(dac_ch, true, hz_mode, current_state_range[ch], current_measure_mode[ch]);

    // 현재 모드 상태 업데이트
    current_force_mode[ch] = hz_mode;
    current_measure_mode[ch] = AD5522::HIGH_Z_MEAS;
}

void PMU::MeasureVolt(int ch) {
    AD5522::Channel dac_ch = (AD5522::Channel)(1 << ch);
    if (current_measure_mode[ch] != AD5522::MV_MODE) {
        PMU_IC.SetChannelMode(dac_ch, true, current_force_mode[ch], current_state_range[ch], AD5522::MV_MODE);
        current_measure_mode[ch] = AD5522::MV_MODE;
    }
}

void PMU::MeasureCurrent(int ch) {
    AD5522::Channel dac_ch = (AD5522::Channel)(1 << ch);
    if (current_measure_mode[ch] != AD5522::MI_MODE) {
        PMU_IC.SetChannelMode(dac_ch, true, current_force_mode[ch], current_state_range[ch], AD5522::MI_MODE);
        current_measure_mode[ch] = AD5522::MI_MODE;
    }
}

void PMU::MeasureTemp(int ch) {
    AD5522::Channel dac_ch = (AD5522::Channel)(1 << ch);
    if (current_measure_mode[ch] != AD5522::TEMP_MODE) {
        PMU_IC.SetChannelMode(dac_ch, true, current_force_mode[ch], current_state_range[ch], AD5522::TEMP_MODE);
        current_measure_mode[ch] = AD5522::TEMP_MODE;
    }
}

void PMU::UpdateMeasurementsFromADC() {
    for (int ch = 0; ch < 4; ch++) {
        // 💡 시스템 전체에서 유일하게 SPI2 통신이 일어나는 안전 구역
        float raw_voltage = ADC_IC.GetVolt(ch);
        float pure_v = raw_voltage - 2.25f;

        if (current_measure_mode[ch] == AD5522::MV_MODE) {
            float pure_voltage = pure_v * 5.0f;
            latestData.voltage[ch] = (pure_voltage * myCalData.v_gain[ch]) + myCalData.v_offset[ch];
        }
        else if (current_measure_mode[ch] == AD5522::MI_MODE) {
            AD5522::CurrentRange current_range = current_state_range[ch];
            float range_resistance = GetRangeResistance(current_range);
            float current_uA_raw = (pure_v / (range_resistance * 2.0f)) * 1000000.0f;
            latestData.current[ch] = current_uA_raw * myCalData.i_gain[ch] + myCalData.i_offset[ch];

            // 기존에 있던 오토레인지(Auto-range) 로직 유지
            float abs_current = (latestData.current[ch] < 0) ? -latestData.current[ch] : latestData.current[ch];
            AD5522::CurrentRange next_range = current_range;

            if (abs_current <= 5.0f) next_range = AD5522::RANGE_5uA;
            else if (abs_current <= 20.0f) next_range = AD5522::RANGE_20uA;
            else if (abs_current <= 200.0f) next_range = AD5522::RANGE_200uA;
            else next_range = AD5522::RANGE_2mA;

            if (current_range != next_range) {
                AD5522::Channel dac_ch = (AD5522::Channel)(1 << ch);
                PMU_IC.SetChannelMode(dac_ch, true, current_force_mode[ch], next_range, current_measure_mode[ch]);
                current_state_range[ch] = next_range;
            }
        }
        else if (current_measure_mode[ch] == AD5522::TEMP_MODE) {
            latestData.temp[ch] = ((pure_v - 1.54f) / 0.0047f) + 25.0f;
        }
    }
}

// 비상 상황 발생 시, 딜레이 및 모드 변경 없이 4개 채널을 즉시 읽어서 latestData 업데이트
void PMU::EmergencyMeasureAll() {
	for (int ch = 0; ch < 4; ch++) {
		float raw_voltage = ADC_IC.GetVolt(ch);
		float pure_v = raw_voltage - 2.25f;

		if (current_measure_mode[ch] == AD5522::MV_MODE) {
			float pure_voltage = pure_v * 5.0f;
			float real_voltage = (pure_voltage * myCalData.v_gain[ch]) + myCalData.v_offset[ch];
			latestData.voltage[ch] = real_voltage;
		}
		else {
			AD5522::CurrentRange current_range = current_state_range[ch];
			float range_resistance = GetRangeResistance(current_range);
			float current_uA_raw = (pure_v / (range_resistance * 2.0f)) * 1000000.0f;
			float calculated_current = current_uA_raw * myCalData.i_gain[ch] + myCalData.i_offset[ch];
			latestData.current[ch] = calculated_current;
		}
	}
}

// 4개 채널 모두 출력 즉시 차단
void PMU::Emergency_Stop() {
	PMU_IC.SetChannelMode(AD5522::ALL, false, AD5522::FV_MODE, AD5522::RANGE_2mA, AD5522::MI_MODE);
}

bool PMU::SaveCalibrationToEEPROM() {
    if (myEEPROM.SaveCalibration(&myCalData)) {
        return true;
    }
    else {
        return false;
    }
}

void PMU::TuneDynamicOffset(int ch) {
    osDelay(pdMS_TO_TICKS(50));

    if (current_force_mode[ch] == AD5522::FV_MODE) {
        MeasureVolt(ch);
        float measured = latestData.voltage[ch];
        float error = target_v[ch] - measured;

        dynamic_v_offset[ch] += (error * 0.5f);

        SetOutputVoltage(ch, target_v[ch]);
    }
    else if (current_force_mode[ch] == AD5522::FI_MODE) {
        MeasureCurrent(ch);
        float measured = latestData.current[ch];
        float error = target_i[ch] - measured;

        dynamic_i_offset[ch] += (error * 0.5f);
        SetOutputCurrent(ch, target_i[ch]);
    }
}

void PMU::ResetDynamicOffset(int ch) {
    dynamic_v_offset[ch] = 0.0f;
    dynamic_i_offset[ch] = 0.0f;
}

void PMU::SetStaticCalV(int ch, float offset, float gain) {
    myCalData.v_offset[ch] = offset;
    myCalData.v_gain[ch] = gain;
}

void PMU::SetStaticCalI(int ch, float offset, float gain) {
    myCalData.i_offset[ch] = offset;
    myCalData.i_gain[ch] = gain;
}


const MuxPins_t PMU::CFF_Pins[4] = {
		{CFF0_A0_GPIO_Port, CFF0_A0_Pin, CFF0_A1_GPIO_Port, CFF0_A1_Pin, CFF0_EN_GPIO_Port, CFF0_EN_Pin},
		{CFF1_A0_GPIO_Port, CFF1_A0_Pin, CFF1_A1_GPIO_Port, CFF1_A1_Pin, CFF1_EN_GPIO_Port, CFF1_EN_Pin},
		{CFF2_A0_GPIO_Port, CFF2_A0_Pin, CFF2_A1_GPIO_Port, CFF2_A1_Pin, CFF2_EN_GPIO_Port, CFF2_EN_Pin},
		{CFF3_A0_GPIO_Port, CFF3_A0_Pin, CFF3_A1_GPIO_Port, CFF3_A1_Pin, CFF3_EN_GPIO_Port, CFF3_EN_Pin}
};

const MuxPins_t PMU::CCOMP_Pins[4] = {

		{CCOMP0_A0_GPIO_Port, CCOMP0_A0_Pin, CCOMP0_A1_GPIO_Port, CCOMP0_A1_Pin, CCOMP0_EN_GPIO_Port, CCOMP0_EN_Pin},
		{CCOMP1_A0_GPIO_Port, CCOMP1_A0_Pin, CCOMP1_A1_GPIO_Port, CCOMP1_A1_Pin, CCOMP1_EN_GPIO_Port, CCOMP1_EN_Pin},
		{CCOMP2_A0_GPIO_Port, CCOMP2_A0_Pin, CCOMP2_A1_GPIO_Port, CCOMP2_A1_Pin, CCOMP2_EN_GPIO_Port, CCOMP2_EN_Pin},
		{CCOMP3_A0_GPIO_Port, CCOMP3_A0_Pin, CCOMP3_A1_GPIO_Port, CCOMP3_A1_Pin, CCOMP3_EN_GPIO_Port, CCOMP3_EN_Pin}
};

// [2] CFF 제어 함수 구현
void PMU::SetCFF(uint8_t ch, uint8_t val) {
    if (ch > 3 || val > 3) return; // 채널 0~3, 값 0~3(A1, A0) 예외 처리

    // 1단계: 안전을 위해 MUX를 먼저 끕니다. (글리치 방지)
    HAL_GPIO_WritePin(CFF_Pins[ch].EN_Port, CFF_Pins[ch].EN_Pin, GPIO_PIN_RESET);

    // 2단계: val 값(0~3)에 따라 A0, A1 핀 설정
    GPIO_PinState a0_state = (val & 0x01) ? GPIO_PIN_SET : GPIO_PIN_RESET; // 1번 비트
    GPIO_PinState a1_state = (val & 0x02) ? GPIO_PIN_SET : GPIO_PIN_RESET; // 2번 비트

    HAL_GPIO_WritePin(CFF_Pins[ch].A0_Port, CFF_Pins[ch].A0_Pin, a0_state);
    HAL_GPIO_WritePin(CFF_Pins[ch].A1_Port, CFF_Pins[ch].A1_Pin, a1_state);

    // 3단계: MUX 켜기
    HAL_GPIO_WritePin(CFF_Pins[ch].EN_Port, CFF_Pins[ch].EN_Pin, GPIO_PIN_SET);
}

// [3] CCOMP 제어 함수 구현 (CFF와 로직은 100% 동일합니다)
void PMU::SetCCOMP(uint8_t ch, uint8_t val) {
    if (ch > 3 || val > 3) return;

    HAL_GPIO_WritePin(CCOMP_Pins[ch].EN_Port, CCOMP_Pins[ch].EN_Pin, GPIO_PIN_RESET);

    GPIO_PinState a0_state = (val & 0x01) ? GPIO_PIN_SET : GPIO_PIN_RESET;
    GPIO_PinState a1_state = (val & 0x02) ? GPIO_PIN_SET : GPIO_PIN_RESET;

    HAL_GPIO_WritePin(CCOMP_Pins[ch].A0_Port, CCOMP_Pins[ch].A0_Pin, a0_state);
    HAL_GPIO_WritePin(CCOMP_Pins[ch].A1_Port, CCOMP_Pins[ch].A1_Pin, a1_state);

    HAL_GPIO_WritePin(CCOMP_Pins[ch].EN_Port, CCOMP_Pins[ch].EN_Pin, GPIO_PIN_SET);
}

void PMU::SetProtection(uint8_t cpolh_mask, uint8_t cl_mask) {
    PMU_IC.SetSystemDefault(cpolh_mask, cl_mask);
}
