//version 1.1
//STM32F1

//#include "stm32f10x_usart.h"

#include "hw.h"
//#include "stm32f10x_gpio.h"

//#include "misc.h" 
#include "uart_hw.h"
#include "uart_config.h"
//#include "led.h"


void UsartHwInit(tUSART* usart)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	USART_ClockInitTypeDef USART_CloclInitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	//GPIO_PinRemapConfig(GPIO_PartialRemap_USART3, ENABLE);

	//partial - tx=pc10/rx=pc11

	USART_RCC_ENABLE;
	
	USART_DeInit(usart->USARTx);
	USART_Cmd(usart->USARTx, DISABLE);

	
	GPIO_PinRemapConfig(usart->GPIO_Rx.GPIO_AF, ENABLE);
	GPIO_PinRemapConfig(usart->GPIO_Tx.GPIO_AF, ENABLE);

	GPIO_InitStructure.GPIO_Pin = usart -> GPIO_Tx.GPIO.GPIO_Pin;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(usart -> GPIO_Tx.GPIO.GPIOx, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = usart -> GPIO_Rx.GPIO.GPIO_Pin;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;

	GPIO_Init(usart -> GPIO_Rx.GPIO.GPIOx, &GPIO_InitStructure);

	
	

	USART_InitStructure.USART_BaudRate = usart -> baudrate;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;

	USART_Init(usart -> USARTx, &USART_InitStructure);
	
	
	USART_ITConfig(usart -> USARTx, USART_IT_RXNE, ENABLE);		
//	USART_ITConfig(usart->USARTx, USART_IT_TC, ENABLE);

	USART_Cmd(usart -> USARTx, ENABLE);

	NVIC_InitStructure.NVIC_IRQChannel = usart->NVIC_IRQChannel;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);


}

void UsartHwSend(tUSART* usart,uint8_t ptr)
{
	USART_SendData(usart -> USARTx, ptr);
}
uint8_t UsartHwRecv(tUSART* usart)
{
	return USART_ReceiveData(usart -> USARTx);
}

uint8_t UsartHwCheckTx(tUSART* usart)
{
	if(USART_GetFlagStatus(usart -> USARTx, USART_FLAG_TC) == RESET)
		return 1;
	else return 0;

}