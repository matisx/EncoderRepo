// version 1.2 #STM32F103

#pragma once

#ifndef __HW_H
#define __HW_H

#define APB1_TIMER_CLOCK 72000000
#define APB2_TIMER_CLOCK 72000000

#include "stm32f10x_conf.h"
#include "stdbool.h"
#include "stm32f10x.h"

#ifdef __cplusplus
extern "C" {
#endif

		
	typedef struct _tGPIO
	{
		GPIO_TypeDef*	GPIOx;
		uint16_t		GPIO_Pin;
	} tGPIO;

	typedef struct _tGPIO_AF
	{
		tGPIO	GPIO;
		uint16_t GPIO_PinSource;
		uint32_t GPIO_AF;
	} tGPIO_AF;

	typedef struct _tEXTI
	{
		uint32_t EXTI_Line;
		uint8_t EXTI_PortSourceGPIOx;
		uint8_t EXTI_PinSourcex;
		IRQn_Type NVIC_IRQChannel;
	} tEXTI;

	typedef struct _tUSART
	{
		USART_TypeDef*   USARTx;
		uint32_t baudrate;
		tGPIO_AF	GPIO_Rx;
		tGPIO_AF	GPIO_Tx;
		IRQn_Type NVIC_IRQChannel;
	} tUSART;
	
	typedef struct _tI2C
	{
		I2C_TypeDef* I2C;
		uint32_t clock;
		uint16_t address;
		tGPIO_AF GPIO_SDA;
		tGPIO_AF GPIO_SCL;
		IRQn_Type NVIC_IRQChannel;
	} tI2C;


	typedef struct _tTimer
	{
		TIM_TypeDef* timer;	// np TIM2;
		uint32_t frequency;	//czêstotliwoœc taktowania timera np 72Mhz
		uint32_t prescaler;	//wpisana zostanie wartoœæ prescaler-1 (dla 8Mhz nale¿y podaæ 8 aby uzyskaæ 1Mhz)
		uint32_t period;	//60*period musi byæ < 0xFFFFFFFF
		uint8_t timerIRQChannel;// np TIM2_IRQn; lub 0; 0 oznacza brak przerwania
		uint32_t overflowFrequnecy; //obliczane na podstawie frequency, prescaler i period

	} tTimer;
	
	typedef enum _tTimerMode
	{
		Down = 0,
		Up   = 1
	} tTimerMode;
	
	typedef struct _tChannelPWM
	{
		tTimer timer;
		uint8_t channelNumber; //wartoœæ 1-4
	} tChannelPWM;
		


	void GpioSet(tGPIO gpio);
	void GpioReset(tGPIO gpio);
	void GpioToggle(tGPIO gpio);
	uint8_t GpioRead(tGPIO gpio);
	void InitOutputDefault(tGPIO gpio);

	void TimerCalculateOverflowFrequncy(tTimer *timer);
	bool TimerSetOverflowFrequency(tTimer *timer, uint32_t overflowFreq);

	void InitTimerDefaultModeUp(tTimer *timer);
	void InitTimerDefaultModeDown(tTimer *timer);
	void InitTimerEx(tTimer *timer, tTimerMode mode, uint8_t prePriority, uint8_t subPriority);
	void InitTimerPWM(tChannelPWM* channel);
	void SetValuePWM(tChannelPWM* channel, uint16_t value);
	
#ifdef __cplusplus
}
#endif


#endif 