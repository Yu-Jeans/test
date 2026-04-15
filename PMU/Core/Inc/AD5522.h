/*
 * AD5522.h
 *
 *  Created on: 2026. 3. 24.
 *      Author: yujin
 */

#ifndef INC_AD5522_H_
#define INC_AD5522_H_

#include "main.h"

class AD5522 {
public:
    // 채널 (다중 선택을 위해 비트 마스크로 설정)
    enum Channel : uint8_t {
        CH0 = 0b0001,
        CH1 = 0b0010,
        CH2 = 0b0100,
        CH3 = 0b1000,
        ALL = 0b1111
    };

    // 출력 모드 (FV / FI)
    enum ForceMode : uint32_t {
        FV_MODE = 0b00,
        FI_MODE = 0b01,
        HIGH_Z_V = 0b10,
        HIGH_Z_I = 0b11
    };

    // 전류 범위 (Range)
    enum CurrentRange : uint32_t {
        RANGE_5uA   = 0b000,
        RANGE_20uA  = 0b001,
        RANGE_200uA = 0b010,
        RANGE_2mA   = 0b011, // 기본값
        RANGE_EXT   = 0b100
    };

    // 측정 대상 (MI / MV) [cite: 1803, 1804, 1805]
    enum MeasureMode : uint32_t {
        MI_MODE = 0b00,
        MV_MODE = 0b01,
        TEMP_MODE = 0b10,
        HIGH_Z_MEAS = 0b11
    };

    enum DAC_Type : uint8_t {
		DAC_FIN = 0b001, // 메인 전압/전류 출력 DAC
		DAC_CLL = 0b010, // 클램프 하한선 DAC
		DAC_CLH = 0b011, // 클램프 상한선 DAC
		DAC_CPL = 0b100, // 비교기 하한선 DAC
		DAC_CPH = 0b101  // 비교기 상한선 DAC
	};

	AD5522(
			SPI_HandleTypeDef* hspi,
			GPIO_TypeDef* syncP, uint16_t syncN,
			GPIO_TypeDef* busyP, uint16_t busyN,
			GPIO_TypeDef* loadP, uint16_t loadN,
			GPIO_TypeDef* resetP, uint16_t resetN);

    bool Init();
	void SetSystemDefault(uint8_t cpolh_mask, uint8_t cl_mask);

    void SetChannelMode(uint8_t ch_mask, bool enable, ForceMode f_mode, CurrentRange range, MeasureMode m_mode);
	void SetForceValue(Channel ch, uint8_t range_bits, float value);
	void SetClamp(Channel ch, DAC_Type type, uint8_t range_bits, float value);
	void SetComparator(Channel ch, DAC_Type type, uint8_t range_bits, float value);

private:
	SPI_HandleTypeDef* hspi;
	GPIO_TypeDef *syncPort;	uint16_t syncPin;
	GPIO_TypeDef *busyPort; uint16_t busyPin;
	GPIO_TypeDef *loadPort;  uint16_t loadPin;
	GPIO_TypeDef *resetPort; uint16_t resetPin;

	void Write29Bits(uint32_t data);
	void WriteDAC(Channel ch, DAC_Type type, uint8_t range_bits, uint16_t dac_code);

	uint16_t CalculateVoltageToDAC(float target_v);
	uint16_t CalculateCurrentToDAC(uint8_t range_bits, float target_i);
	float GetRsenseValue(uint8_t range_bits);
};




#endif /* INC_AD5522_H_ */
