//version 1.0
//STM32F1

#pragma once

#ifndef __I2C_COMMON_H
#define __I2C_COMMON_H

#include "hw.h"
#include "stdint.h"
#include "i2c_hw.h"
#include "i2c_config.h"

#ifdef __cplusplus
extern "C" {
#endif
	
	typedef struct
	{
		tI2C* I2C_Hw;
	} cI2C;

	void cI2C_Init(cI2C* i2c);
	
	uint8_t I2C_Read(tI2C* I2Cx, uint8_t address, uint8_t reg);
	uint8_t	I2C_ReadMulti(tI2C* I2Cx, uint8_t address, uint8_t reg, uint8_t* data, uint16_t count); //zwraca 0 jeœli wszystko ok
	uint8_t I2C_ReadNoRegister(tI2C* I2Cx, uint8_t address);
	uint8_t	I2C_ReadMultiNoRegister(tI2C* I2Cx, uint8_t address, uint8_t* data, uint16_t count);
	
	void I2C_Write(tI2C* I2Cx, uint8_t address, uint8_t reg, uint8_t data);
	void I2C_WriteMulti(tI2C* I2Cx, uint8_t address, uint8_t reg, uint8_t* data, uint16_t count);
	void I2C_WriteNoRegister(tI2C* I2Cx, uint8_t address, uint8_t data);
	void I2C_WriteMultiNoRegister(tI2C* I2Cx, uint8_t address, uint8_t* data, uint16_t count);
	
	uint8_t I2C_IsDeviceConnected(tI2C* I2Cx, uint8_t address);
	
#ifdef __cplusplus
}
#endif


#endif 