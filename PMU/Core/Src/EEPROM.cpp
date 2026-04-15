/*
 * EEPROM.cpp
 *
 *  Created on: 2026. 3. 20.
 *      Author: yujin
 */

#include "EEPROM.h"
#include "FreeRTOS.h"
#include "task.h"

EEPROM24FC064::EEPROM24FC064(I2C_HandleTypeDef* i2c, uint16_t addr)
    : hi2c(i2c), devAddress(addr) {}

bool EEPROM24FC064::Init() {

    HAL_StatusTypeDef status = HAL_I2C_IsDeviceReady(hi2c, devAddress, 3, 10);

    if (status == HAL_OK) {
        return true;
    } else {
        return false;
    }
}

// 1. EEPROM에 캘리브레이션 32바이트 통째로 굽기
bool EEPROM24FC064::SaveCalibration(CalibrationData_t* data){
    HAL_StatusTypeDef status = HAL_I2C_Mem_Write(
        hi2c,
        devAddress,
        0x0000,
        I2C_MEMADD_SIZE_16BIT,
        (uint8_t*)data,
        sizeof(CalibrationData_t),
        1000
    );

    if (status != HAL_OK) //EEPROM 칩에 데이터를 굽는 데 성공했을 때만 10ms 쉬도록 만듦
    	{
    		return false;
    	}


    vTaskDelay(pdMS_TO_TICKS(10));
//FreeRTOS 설정에 따라 1번 심장이 뛰는(1 Tick)데 걸리는 시간이 1ms일 수도 있고 10ms일 수도 있음. 실제 시간 10ms에 해당하는 틱수로 알아서 계산해 줌

    return true;
}

// 2. EEPROM에서 캘리브레이션 32바이트 통째로 읽어오기
bool EEPROM24FC064::LoadCalibration(CalibrationData_t* data) {
    HAL_StatusTypeDef status = HAL_I2C_Mem_Read(
        hi2c,
        devAddress,
        0x0000,
        I2C_MEMADD_SIZE_16BIT,
        (uint8_t*)data,
        sizeof(CalibrationData_t),
        1000
    );

    return (status == HAL_OK);
}
