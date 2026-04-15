/*
 * ADS131A04.h
 *
 *  Created on: 2026. 3. 19.
 *      Author: yujin
 */
/*
#define ADC_CS_GPIO_Port GPIOB
#define ADC_CS_Pin       GPIO_PIN_4

#include "main.h"

class ADS131A04IPBSR{
private:
	uint32_t frame_data[6];
	void SpiFrameTransfer(uint32_t command);
public:
	ADS131A04IPBSR();
	~ADS131A04IPBSR();

	bool Init();
	bool Send_UNLOCK();
	uint32_t GetRawValue(uint8_t channel);
	float GetVolt(uint8_t channel);
};
*/

#ifndef INC_ADS131A04_H_
#define INC_ADS131A04_H_

#include "main.h"

class ADS131A04IPBSR{
private:
	SPI_HandleTypeDef* hspi;
    GPIO_TypeDef* csPort;
    uint16_t csPin;
    GPIO_TypeDef* resetPort;
	uint16_t resetPin;

    uint32_t frame_data[6];
	void SpiFrameTransfer(uint32_t command);
public:
	ADS131A04IPBSR(SPI_HandleTypeDef* spi, GPIO_TypeDef* csP, uint16_t csN, GPIO_TypeDef* resetP, uint16_t resetN);
	~ADS131A04IPBSR();

	bool Init();
	bool Send_UNLOCK();
	uint32_t GetRawValue(uint8_t channel);
	float GetVolt(uint8_t channel);
	void TriggerSync();
};

#endif /* INC_ADS131A04_H_ */
