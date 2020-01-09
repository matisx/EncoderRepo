//version 1.0
//STM32F1

#pragma once

#ifndef __I2C_HW_H
#define __I2C_HW_H

#include "hw.h"
#include "stdint.h"

/* Private defines */
#define I2C_TRANSMITTER_MODE   0
#define I2C_RECEIVER_MODE      1
#define I2C_ACK_ENABLE         1
#define I2C_ACK_DISABLE        0

#ifdef __cplusplus
extern "C" {
#endif
	
		/* Hardware functions */
	void	I2C_Hw_Init(tI2C* hw);
	uint8_t I2C_Hw_Start(tI2C* I2Cx, uint8_t address, uint8_t direction, uint8_t ack);
	uint8_t	I2C_Hw_WriteData(tI2C* I2Cx, uint8_t data);
	uint8_t I2C_Hw_ReadAck(tI2C* I2Cx);
	uint8_t I2C_Hw_ReadNack(tI2C* I2Cx);
	uint8_t I2C_Hw_Stop(tI2C* I2Cx);
		
	void I2C_SetLowTimeout();
	void I2C_SetHighTimeout();

		
		
#ifdef __cplusplus
}
#endif


#endif 
