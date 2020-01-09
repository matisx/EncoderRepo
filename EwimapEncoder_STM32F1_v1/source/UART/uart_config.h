//version 1.1
//STM32F1

#pragma once

#ifndef __USART_HW_H
#define __USART_HW_H


#include "hw.h"

#ifdef __cplusplus
extern "C" {
#endif

#define USART_RCC_ENABLE RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
#define USART_IT_FUNCTION USART1_IRQHandler


static tUSART Uart1Hw = 
{
		
	{ USART1 },
	{ 115200 },
	{ GPIOA, GPIO_Pin_10, GPIO_PinSource10, 0 },// GPIO_PartialRemap_USART3 }, //Rx
	{ GPIOA, GPIO_Pin_9,  GPIO_PinSource9,  0 },//GPIO_PartialRemap_USART3}, //Tx
	{ USART1_IRQn }
};



#ifdef __cplusplus
}
#endif


#endif 