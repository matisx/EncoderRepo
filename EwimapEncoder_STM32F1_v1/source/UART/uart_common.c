//version 1.1
//STM32F1

#include "uart_common.h"

void cUsartRxIT(cUSART* usart)
{
	uint8_t data = UsartHwRecv(usart -> hw);
	if (usart -> rxCounter == 0)
	{
		usart -> rxBuffer[usart -> rxIndex] = data;
		
		if (data == '\r')
		{
			usart->rxBuffer[usart->rxIndex] = 0; //koniec stringa
			usart -> rxCounter++;
			usart -> rxIndex = 0;
		}
		else
		{
			if (++usart -> rxIndex == USART_RX_BUFFER_SIZE)
			{//jeœli komenda jest za d³uga zerujemy wszystko
				usart -> rxCounter = 0;
				usart -> rxIndex = 0;
			}
		}
	}
}

bool cUsartCheckRx(cUSART* usart)
{
	if (usart->rxCounter == 0)
		return false;
	else
		return true;
}

char* cUsartRxData(cUSART* usart)
{
	if (cUsartCheckRx(usart))
	{
		usart->rxIndex = 0;
		usart->rxCounter = 0;
		return usart->rxBuffer;

	}
	else
		return 0;
}

void cUsartTxIT(cUSART* usart)
{
	if (usart -> txGetIndex != usart -> txPutIndex)
	{
		UsartHwSend(usart -> hw, usart -> txBuffer[usart -> txGetIndex]);
		while(UsartHwCheckTx(usart -> hw) != 1)
		{
		}
		if (++usart -> txGetIndex == USART_TX_BUFFER_SIZE)
		{
			usart -> txGetIndex = 0;
		}
	}
}

void cUsartInit(cUSART* usart)
{
	UsartHwInit(usart -> hw);
	usart -> rxIndex = 0;
	usart -> txGetIndex = 0;
	usart -> txPutIndex = 0;
	usart -> rxCounter = 0;
}

void cUsartSend(cUSART *usart, char* c)
{

//	char* ptr = c;
//	while (*ptr != 0)
//	{
//		usart -> txBuffer[usart -> txPutIndex] = (*ptr);
//		ptr++;
//		if (++usart -> txPutIndex == USART_TX_BUFFER_SIZE)
//		{
//			usart -> txPutIndex = 0;
//		}
//	}
//	cUsartTxIT(usart);
	uint8_t* ptr = c;
	while (*ptr != 0)
	{		
		while (UsartHwCheckTx(usart->hw))
		{
		}
		
		UsartHwSend(usart->hw, *ptr);
		
		ptr++;
	}
}

void cUsartSendCnt(cUSART *usart, uint8_t* c, uint16_t cnt)
{
	uint8_t* ptr = c;
	while (cnt-- != 0)
	{		
		UsartHwSend(usart->hw, *ptr);

		while (UsartHwCheckTx(usart->hw))
		{
		}
		ptr++;
	}
}


/*
void print(char* c)
{

	char* ptr = c;
	while (*ptr != 0)
	{
		UsartSend(*ptr);
		ptr++;
		while (UsartCheckTx())
		{ }
	}
}

*/