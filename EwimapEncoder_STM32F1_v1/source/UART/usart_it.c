//version 1.1
//STM32F1

#include "uart.h"
#include "uart_config.h"


extern cUSART UartDebug;

void USART_IT_FUNCTION()
{	//TODO: usart ze struktury, nie na sta³e
	if (USART_GetITStatus(UartDebug.hw->USARTx, USART_IT_RXNE) != RESET)
	{
		cUsartRxIT(&UartDebug);
		USART_ClearITPendingBit(UartDebug.hw->USARTx, USART_IT_RXNE); //???

	}

//	if (USART_GetITStatus(UartDebug.hw->USARTx, USART_IT_TC) != RESET)
//	{
//		cUsartTxIT(&UartDebug);
//		USART_ClearITPendingBit(UartDebug.hw->USARTx, USART_IT_TC);
//	}
}