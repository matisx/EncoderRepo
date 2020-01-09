//version 1.1
//STM32F1

#include "uart.h"
#include "uart_common.h"
#include "uart_config.h"

cUSART UartDebug =
{
	{ &Uart1Hw },
	0, 0, 0, 0, 0, 0
};
void print(char* c)
{
	cUsartSend(&UartDebug, c);
}

void usartSend(uint8_t* buf, uint16_t cnt)
{
	cUsartSendCnt(&UartDebug, buf, cnt);
}
	
void UsartInit(void)
{
	cUsartInit(&UartDebug);
}

char* UsartRxData(void)
{
	return cUsartRxData(&UartDebug);
}