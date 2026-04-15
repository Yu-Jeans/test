/*
 * PMU.h
 *
 *  Created on: 2026. 3. 24.
 *      Author: yujin
 */

#ifndef INC_PMU_H_
#define INC_PMU_H_

#include "ADS131A04.h"
#include "EEPROM.h"
#include "AD5522.h"
#include "stdio.h"

typedef enum {
    SYS_NORMAL = 0,
    SYS_ALARM_TEMP,
    SYS_ALARM_CGALM
} PMU_SystemStatus_t;

typedef struct {
    float voltage[4];
    float current[4];
    float temp[4];

	uint8_t comparator_status;
	PMU_SystemStatus_t system_status;
} PMU_Data_t;

typedef enum {
    CMD_FORCE_VOLTAGE,
    CMD_FORCE_CURRENT,
	CMD_HIGH_Z_V,
    CMD_HIGH_Z_I,
    CMD_MEAS_VOLTAGE,
    CMD_MEAS_CURRENT,
	CMD_MEAS_TEMP,
    CMD_EMERGENCY_STOP,
	CMD_SAVE_CALIBRATION,
	CMD_TUNE_DYNAMIC,
    CMD_SET_CFF,
    CMD_SET_CCOMP
} PMU_CmdType_t;

typedef struct {
    PMU_CmdType_t cmd_type;
    uint8_t channel;
    float value;
    uint8_t mux_val;
} PMU_CmdPacket_t;

typedef struct {
    GPIO_TypeDef* A0_Port; uint16_t A0_Pin;
    GPIO_TypeDef* A1_Port; uint16_t A1_Pin;
    GPIO_TypeDef* EN_Port; uint16_t EN_Pin;
} MuxPins_t;

class PMU{
private:
	ADS131A04IPBSR ADC_IC; //ADC는 PMU 안(예를 들면 PMU의 동작을 정의하는 곳)에서만 사용 가능
	AD5522         PMU_IC;
	EEPROM24FC064  myEEPROM;
	CalibrationData_t myCalData;

    static const MuxPins_t CFF_Pins[4];
    static const MuxPins_t CCOMP_Pins[4];

    float target_v[4] = {0.0f, };
	float target_i[4] = {0.0f, };
	float dynamic_v_offset[4] = {0.0f, };
	float dynamic_i_offset[4] = {0.0f, };

public:
	PMU(SPI_HandleTypeDef* hspi_adc,
		GPIO_TypeDef* csPort_adc, uint16_t csPin_adc,
		GPIO_TypeDef* resetPort_adc, uint16_t resetPin_adc,

		SPI_HandleTypeDef* hspi_pmu,
		GPIO_TypeDef* syncP_pmu, uint16_t syncN_pmu,
		GPIO_TypeDef* busyP_pmu, uint16_t busyN_pmu,
		GPIO_TypeDef* loadP_pmu, uint16_t loadN_pmu,
		GPIO_TypeDef* resetP_pmu, uint16_t resetN_pmu,

		I2C_HandleTypeDef* hi2c_eeprom, uint16_t addr_eeprom);
	~PMU();

	AD5522::CurrentRange current_state_range[4];
	AD5522::ForceMode    current_force_mode[4];
    AD5522::MeasureMode  current_measure_mode[4];

	PMU_Data_t latestData;
	bool Init();
	void Loop();

	float GetRangeResistance(AD5522::CurrentRange range);

	void SetOutputVoltage(int ch, float target_volt);
	void SetOutputCurrent(int ch, float current_uA);

	void SetHighZ(int ch, AD5522::ForceMode hz_mode);

	void MeasureVolt(int ch);
	void MeasureCurrent(int ch);
	void MeasureTemp(int ch);

	void UpdateMeasurementsFromADC();

	void Emergency_Stop();
	void EmergencyMeasureAll();

	bool SaveCalibrationToEEPROM();
	void TuneDynamicOffset(int ch);
	void ResetDynamicOffset(int ch);
	void SetStaticCalV(int ch, float offset, float gain);
    void SetStaticCalI(int ch, float offset, float gain);

	void SetCFF(uint8_t ch, uint8_t val);
	void SetCCOMP(uint8_t ch, uint8_t val);

	void SetProtection(uint8_t cpolh_mask, uint8_t cl_mask);
};
#endif /* INC_PMU_H_ */
