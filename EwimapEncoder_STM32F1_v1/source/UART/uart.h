//version 1.1
//STM32F1

#pragma once

#ifndef __UART_H
#define __UART_H

#include "uart_common.h"


#ifdef __cplusplus
extern "C" {
#endif


	void print(char* c);
	void UsartProcessData();
	void UsartInit(void);
	void usartSend(uint8_t* buf, uint16_t cnt);
	char* UsartRxData(void);

#ifdef __cplusplus
}
#endif


#endif 