// version 1.2 #STM32F103
#include "hw.h"
//============GPIO============
#pragma region GPIO

inline void GpioSet(tGPIO gpio)
{
	gpio.GPIOx->BSRR = gpio.GPIO_Pin;
}
inline void GpioReset(tGPIO gpio)
{
	gpio.GPIOx->BRR = gpio.GPIO_Pin;

}
inline void GpioToggle(tGPIO gpio)
{
	if (gpio.GPIOx->ODR & gpio.GPIO_Pin)
	{
		GpioReset(gpio);
	}
	else
	{
		GpioSet(gpio);	
	}
}
inline uint8_t GpioRead(tGPIO gpio)
{
	return gpio.GPIOx->IDR & gpio.GPIO_Pin;
	//return GPIO_ReadInputDataBit(gpio.GPIOx, gpio.GPIO_Pin);
}
void InitOutputDefault(tGPIO gpio)
{	
	GPIO_InitTypeDef GPIO_InitStructure;
  
	GPIO_InitStructure.GPIO_Pin = gpio.GPIO_Pin;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(gpio.GPIOx, &GPIO_InitStructure);
	
	//gpio.GPIOx->CRL = 

}

#pragma endregion


//============TIMER============
#pragma region TIMER

void TimerCalculateOverflowFrequncy(tTimer *timer)
{
	if (timer->prescaler == 0 || timer->period == 0)
	{
		timer->overflowFrequnecy = 0;
	}
	else
	{
		timer->overflowFrequnecy = timer->frequency / (timer->prescaler * timer->period);
	}
}
bool TimerSetOverflowFrequency(tTimer *timer, uint32_t overflowFreq)
{
	uint32_t temp=0;
	if (overflowFreq == 0)
	{
		temp = 0;
	}
	else
	{
		temp = timer->frequency / (overflowFreq*timer->prescaler);
	}
	
	if (temp < 0xFFFF) //timer 16 bitowy
	{
		timer->overflowFrequnecy = overflowFreq;
		timer->period = temp;
		TIM_SetAutoreload(timer->timer, timer->period);
		
		temp = TIM_GetCounter(timer->timer);
		if (temp >= timer->period)
		{
			temp = temp % timer->period;
			TIM_SetCounter(timer->timer, timer->period - 2);
		}
		
		
		return true;
	}
	else
		return false;
}

void InitTimerDefaultModeUp(tTimer *timer)
{
	InitTimerEx(timer, Up, 2, 2);
}
void InitTimerDefaultModeDown(tTimer *timer)
{
	InitTimerEx(timer, Down, 2, 2);
}
void InitTimerEx(tTimer *timer, tTimerMode mode ,uint8_t prePriority, uint8_t subPriority)
{
	if (timer->timer == 0)
		return;
	
	TimerCalculateOverflowFrequncy(timer);
	
	TIM_TimeBaseInitTypeDef Timer_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	Timer_InitStructure.TIM_Period = timer->period-1; //TODO: -1 ??????
	Timer_InitStructure.TIM_Prescaler = timer->prescaler-1;  //dla 1 wpisuje 0, czyli brak skalowania
	Timer_InitStructure.TIM_RepetitionCounter = 0;
	if (mode == Up)
	{
		Timer_InitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	}
	else if(mode == Down)
	{
		Timer_InitStructure.TIM_CounterMode = TIM_CounterMode_Down;
	}
	TIM_TimeBaseInit(timer->timer, &Timer_InitStructure);
	
	if (timer->timerIRQChannel != 0)
	{
		NVIC_InitStructure.NVIC_IRQChannel = timer->timerIRQChannel;
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = prePriority;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = subPriority;
		NVIC_Init(&NVIC_InitStructure);
						
		TIM_ITConfig(timer->timer, TIM_IT_Update, ENABLE);
	}
	
	TIM_Cmd(timer->timer, ENABLE);
}

void InitTimerPWM(tChannelPWM *channel)
{
	TimerCalculateOverflowFrequncy(&channel->timer);

	
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;

	TIM_TimeBaseInitStructure.TIM_Period = channel->timer.period-1;
	TIM_TimeBaseInitStructure.TIM_Prescaler = channel->timer.prescaler-1;
	
	TIM_TimeBaseInit(channel->timer.timer, &TIM_TimeBaseInitStructure);
	
	
	TIM_OCInitTypeDef TIM_OCInitStructure;
	
	TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Reset;
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_Pulse = 0;
	
	if (channel->channelNumber == 1)
	{
		TIM_OC1Init(channel->timer.timer, &TIM_OCInitStructure);
		TIM_OC1PreloadConfig(channel->timer.timer, TIM_OCPreload_Enable);
	}
	else if (channel->channelNumber == 2)
	{
		TIM_OC2Init(channel->timer.timer, &TIM_OCInitStructure);
		TIM_OC2PreloadConfig(channel->timer.timer, TIM_OCPreload_Enable);
	}
	else if (channel->channelNumber == 3)
	{
		TIM_OC3Init(channel->timer.timer, &TIM_OCInitStructure);
		TIM_OC3PreloadConfig(channel->timer.timer, TIM_OCPreload_Enable);
	}
	else if (channel->channelNumber == 4)
	{
		TIM_OC4Init(channel->timer.timer, &TIM_OCInitStructure);
		TIM_OC4PreloadConfig(channel->timer.timer, TIM_OCPreload_Enable);
	}
	else
	{
		//b³¹d
	}
	
	TIM_Cmd(channel->timer.timer, ENABLE);
}

void SetValuePWM(tChannelPWM* channel, uint16_t value)
{
	if (channel->channelNumber == 1)
	{
		TIM_SetCompare1(channel->timer.timer, value);
	}
	else if (channel->channelNumber == 2)
	{
		TIM_SetCompare2(channel->timer.timer, value);
	}
	else if (channel->channelNumber == 3)
	{
		TIM_SetCompare3(channel->timer.timer, value);
	}
	else if (channel->channelNumber == 4)
	{
		TIM_SetCompare4(channel->timer.timer, value);
	}
}
#pragma endregion


