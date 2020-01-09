//version 1.0
//STM32F1

#pragma once

#ifndef __I2C_HW_CONFIG_H
#define __I2C_HW_CONFIG_H

#include "hw.h"
#include "stdint.h"

#ifdef __cplusplus
extern "C" {
#endif
	
	
	#define I2C_RCC_ENABLE RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE)
//#define USART_IT_FUNCTION USART3_IRQHandler


	

#ifdef __cplusplus
}
#endif


static tI2C I2C_Hw1 = 
{ 
	I2C1,
	300000,	//clock 400000
	0x00,	//address
	{{ GPIOB, GPIO_Pin_7}, GPIO_PinSource7, 0 }, //SDA
	{{ GPIOB, GPIO_Pin_6}, GPIO_PinSource6, 0 }, //SCL
	I2C1_EV_IRQn
};
static tI2C I2C_Hw2 = 
{ 
	I2C2,
	200000,	//clock 400000
	0x00,	//address
	{{ GPIOB, GPIO_Pin_11}, GPIO_PinSource11, 0 }, //SDA
	{{ GPIOB, GPIO_Pin_10}, GPIO_PinSource10, 0 }, //SCL
	I2C2_EV_IRQn
};

#endif 