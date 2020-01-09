//version 1.1
//STM32F1

#pragma once

#ifndef __UART_HW_H
#define __UART_HW_H

#include "hw.h"
#include "stdint.h"

#ifdef __cplusplus
extern "C" {
#endif



	void UsartHwInit(tUSART* usart);
	void UsartHwSend(tUSART* usart, uint8_t ptr);
	uint8_t UsartHwCheckTx(tUSART* usart);
	

#ifdef __cplusplus
}
#endif


#endif