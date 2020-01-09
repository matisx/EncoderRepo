#include <stm32f10x_gpio.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x_tim.h>
#include <stm32f10x_spi.h>
#include "i2c_common.h"
#include "hw.h"
#include "uart.h"

#define TIM_ENCODER TIM4
#define TIM_ENCODER_OVERFLOW_VALUE 64000
#define TIM_ENCODER_RCC RCC_APB1Periph_TIM4
#define PORT_ENCODER GPIOB
#define PORT_ENCODER_RCC RCC_APB2Periph_GPIOB
#define PORT_UART_RCC RCC_APB2Periph_GPIOA
#define SPI_RPI_PORT_RCC RCC_APB2Periph_GPIOB
//#define SPI_RPI_RCC RCC_APB2Periph_SPI1
#define SPI_RPI_RCC RCC_APB1Periph_SPI2
#define SPI_RPI SPI2 // SPI1
#define SPI_RPI_IRQ SPI2_IRQn //SPI1_IRQn

#define ENCODER_PULSES_PER_REV 800

#define USE_LCD 1 

uint8_t CRC8(uint8_t *inData, uint8_t len);
uint8_t gencrc8(uint8_t *data, uint16_t len);
uint8_t Crc8(const void* vptr, int len);

void InitRCC(void);
static uint8_t InitPeriph();
void Delay()
{
	int i;
	for (i = 0; i < 300000; i++)
		asm("nop");
}

volatile int32_t encoderValue = 0;
volatile int32_t encoderRev = -1; //-1 because first event is generated by init function to update autoreload register etc.
volatile uint32_t encoderTimerValue = 0;
volatile float encoderPosition = 0.0f;


tI2C I2C2_Hw = 
{ 
	I2C2,
	100000,	//clock 400000
	0xAA,	//address
	{ { GPIOB, GPIO_Pin_11 }, GPIO_PinSource11, 0 }, //SDA
	{ { GPIOB, GPIO_Pin_10 }, GPIO_PinSource10, 0 }, //SCL
	I2C2_EV_IRQn
};

cI2C I2C_2
{
	&I2C2_Hw
};

volatile uint8_t i2cConnected = 0;
uint8_t rxData[64];
uint8_t i = 0, txData[64] = { 12, 34, 56, 78, 90 };
uint8_t rxIndex=0, txIndex=0;



#define HD44780_RS_COMMAND 0x00
#define HD44780_RS_DATA 0x01

#define HD44780_LIGHT_ON 0x01
#define HD44780_LIGHT_OFF 0x00

#define HD44780_PIN_RS 0
#define HD44780_PIN_RW 1
#define HD44780_PIN_ENABLE 2
#define HD44780_PIN_LIGHT 3
#define HD44780_PIN_4BIT_DATA 0xF0


typedef struct 
{
	uint8_t Light;
	uint8_t I2C_Address;
	
	
} HD44780_HW;


void HD44780_HW_SendData4Bit(HD44780_HW *display, uint8_t RS, uint8_t data)
{
	data = (data & 0x0F) << 4;
	
	data &= ~(0x01 << HD44780_PIN_RW); //WRITE
	data |= (0x01 << HD44780_PIN_ENABLE);
	if (RS == HD44780_RS_COMMAND)
	{
		data &= ~(0x01 << HD44780_PIN_RS);
	}
	else
	{
		data |= (0x01 << HD44780_PIN_RS);
	}
	
	if (display->Light == HD44780_LIGHT_OFF)
	{
		data &= ~(0x01 << HD44780_PIN_LIGHT);
	}
	else
	{
		data |= (0x01 << HD44780_PIN_LIGHT);
	}
	
	I2C_WriteNoRegister(&I2C2_Hw, display->I2C_Address, data); // 0xAA = 0b10101010

	Delay();//!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	data &= ~(0x01 << HD44780_PIN_ENABLE);
	I2C_WriteNoRegister(&I2C2_Hw, display->I2C_Address, data); // 0xAA = 0b10101010
	Delay();//!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	
}

void HD44780_HW_Put(HD44780_HW* display, char c)
{
	HD44780_HW_SendData4Bit(display, HD44780_RS_DATA, ((c & 0xF0)>>4) );
	HD44780_HW_SendData4Bit(display, HD44780_RS_DATA, (c & 0x0F));
}

HD44780_HW dispplayI2c = { HD44780_LIGHT_ON, 0x7F };






int main()
{	
	uint8_t currAddress = 0;
	tGPIO portLed = { GPIOC, GPIO_Pin_13 };
	
	InitRCC();
	
	InitOutputDefault(portLed);	

		  /* 1 bit for pre-emption priority, 3 bits for subpriority */
	//NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
	
	I2C_Hw_Init(&I2C2_Hw);
	UsartInit();
	
	Delay();

	InitPeriph();
	
	/*
	for (currAddress = 0x10; currAddress < 0xEF; currAddress++)
	{
		if (I2C_IsDeviceConnected(&I2C2_Hw, currAddress) == 0)
		{
			//i2cConnected = 0;
		}
		else
		{
			i2cConnected = currAddress; //0x7F
		}
	}
	*/
	

#pragma region ENCODER
				  
	//#####################################   ENCODER
    
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;// GPIO_Mode_IN_FLOATING; //GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;    
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_Init(PORT_ENCODER, &GPIO_InitStructure);
    
	//GPIO_PinRemapConfig()
    
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_TimeBaseStructure.TIM_Period = TIM_ENCODER_OVERFLOW_VALUE - 1;
	TIM_TimeBaseStructure.TIM_Prescaler = 0;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    
	TIM_TimeBaseInit(TIM_ENCODER, &TIM_TimeBaseStructure);
    
	/* Configure the timer */
	TIM_EncoderInterfaceConfig(TIM_ENCODER, TIM_EncoderMode_TI12, TIM_ICPolarity_Rising, TIM_ICPolarity_Rising);
    
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
	NVIC_Init(&NVIC_InitStructure);
						
	TIM_ITConfig(TIM_ENCODER, TIM_IT_Update, ENABLE);
	
	
	/* TIM4 counter enable */
	TIM_Cmd(TIM_ENCODER, ENABLE);
	
	//#####################################   ENCODER
#pragma endregion
	
	uint8_t displayConnected = 0u;
	//displayConnected = I2C_IsDeviceConnected(&I2C2_Hw, 0x7E);  //returns 1 
	
	
	//I2C_WriteNoRegister(&I2C2_Hw, 0x7E, 0xAA); // 0xAA = 0b10101010
		
	print("Start EwimapEncoder\n");
		
	//SPI_I2S_SendData(SPI_RPI, txData[txIndex]);
	//txIndex++;
	HD44780_HW_SendData4Bit(&dispplayI2c, HD44780_RS_COMMAND, 0x03); //mo first command is 8bit
	
	HD44780_HW_SendData4Bit(&dispplayI2c, HD44780_RS_COMMAND, 0x03);
	HD44780_HW_SendData4Bit(&dispplayI2c, HD44780_RS_COMMAND, 0x03);
	
	HD44780_HW_SendData4Bit(&dispplayI2c, HD44780_RS_COMMAND, 0x02);	//mo FunctionSet DL=0(4-bit mode)
	
	HD44780_HW_SendData4Bit(&dispplayI2c, HD44780_RS_COMMAND, 0x02);	//mo FunctionSet DL=0(4-bit mode)
	HD44780_HW_SendData4Bit(&dispplayI2c, HD44780_RS_COMMAND, 0x08); //mo N=1 (2 lines) F=0 (5x8 dots)
	
	HD44780_HW_SendData4Bit(&dispplayI2c, HD44780_RS_COMMAND, 0x00);//mo DisplayOff
	HD44780_HW_SendData4Bit(&dispplayI2c, HD44780_RS_COMMAND, 0x08);
	
	HD44780_HW_SendData4Bit(&dispplayI2c, HD44780_RS_COMMAND, 0x0);//mo DisplayOn
	//HD44780_HW_SendData4Bit(&dispplayI2c, HD44780_RS_COMMAND, 0xC); //C=0, B=0 => cursor off, not blinking
	HD44780_HW_SendData4Bit(&dispplayI2c, HD44780_RS_COMMAND, 0xF); //C=1, B= => cursor on, blinking

	HD44780_HW_SendData4Bit(&dispplayI2c, HD44780_RS_COMMAND, 0x0); //mo EntryMode 
	HD44780_HW_SendData4Bit(&dispplayI2c, HD44780_RS_COMMAND, 0x6); //mo I/D=1(Increment), S=0
	
	HD44780_HW_SendData4Bit(&dispplayI2c, HD44780_RS_COMMAND, 0x0); //mo ClearDisplay 0x01
	HD44780_HW_SendData4Bit(&dispplayI2c, HD44780_RS_COMMAND, 0x1);
	
	HD44780_HW_SendData4Bit(&dispplayI2c, HD44780_RS_COMMAND, 0x0); //mo ReturnHome 0x02
	HD44780_HW_SendData4Bit(&dispplayI2c, HD44780_RS_COMMAND, 0x2);


	
	//text
	char c = 'A';
	uint16_t cnt = 0;
	for (cnt = 0; cnt < 80; cnt++)
	{
		HD44780_HW_Put(&dispplayI2c, c++);
	}
	
	for (;;)
	{	
		
		//GpioToggle(portLed);
		//TODO disable interrupts
		encoderTimerValue = TIM_GetCounter(TIM_ENCODER);
		encoderValue = encoderRev*TIM_ENCODER_OVERFLOW_VALUE + (int32_t) encoderTimerValue;
		encoderPosition = encoderValue / (float) ENCODER_PULSES_PER_REV;		
		//TODO eneable interrupts
		
		print(">\n"); //works
		
		for (i = 0; i < 4; i++)
		{	
			GpioReset(portLed);
			//while (SPI_I2S_GetFlagStatus(SPI_RPI, SPI_I2S_FLAG_TXE) == RESET)
			//	;
			GpioSet(portLed);
			//SPI_I2S_SendData(SPI_RPI, txData[i]);
		}
		//while (SPI_I2S_GetFlagStatus(SPI_RPI, SPI_I2S_FLAG_TXE) == RESET)
		//	;
		//SPI_TransmitCRC(SPI_RPI);
		
		uint8_t crcCalculated = CRC8(txData, 4);
		uint8_t crc8Calculated = gencrc8(txData, 4);
		uint8_t  crc8 = Crc8(txData, 4);   //This algorithm return the same result as Stm32 SPI CRC 
		/*
		while (SPI_I2S_GetFlagStatus(SPI_RPI, SPI_I2S_FLAG_RXNE) == RESET)
			;
		recvData = SPI_I2S_ReceiveData(SPI_RPI);
		*/
		
		Delay();

	}
}

void InitRCC(void)
{
	
	RCC_APB2PeriphClockCmd(PORT_UART_RCC, ENABLE); //GPIOA
	RCC_APB2PeriphClockCmd(PORT_ENCODER_RCC, ENABLE); //GPIOB
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	RCC_APB1PeriphClockCmd(TIM_ENCODER_RCC, ENABLE);
	//RCC_APB2PeriphClockCmd(SPI_RPI_RCC, ENABLE);
	RCC_APB1PeriphClockCmd(SPI_RPI_RCC, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
}

static uint8_t InitPeriph()
{
	GPIO_InitTypeDef GPIO_InitStructure;

	
	/*
	GPIO_PinRemapConfig(GPIO_Remap_SPI2, ENABLE);
	*/
	
	//Confugure SCK and MOSI pins as Input Floating 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15 | GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	// Confugure MISO pin as Alternate Function Push Pull 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	//Confugure NSS pin as ..................... 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;  //GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	
	NVIC_InitTypeDef NVIC_InitStructure;
	  /* Configure and enable SPI_SLAVE interrupt --------------------------------*/
	NVIC_InitStructure.NVIC_IRQChannel = SPI_RPI_IRQ;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	SPI_I2S_DeInit(SPI_RPI);

		  	// Konfiguracja SPI1
	SPI_InitTypeDef SPI_InitStructure;
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;// SPI_Direction_2Lines_RxOnly; // SPI_Direction_1Line_Rx; //;// SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Slave;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low ;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
		// NSS sprzetowo
	SPI_InitStructure.SPI_NSS = SPI_NSS_Hard; //SPI_NSS_Soft
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	
	SPI_Init(SPI_RPI, &SPI_InitStructure);
	
	SPI_CalculateCRC(SPI_RPI, ENABLE);

	//SPI_I2S_ITConfig(SPI_RPI, SPI_I2S_IT_RXNE, ENABLE);
	SPI_I2S_ITConfig(SPI_RPI, SPI_I2S_IT_TXE, ENABLE);

	  	// Wlacz NSS
	SPI_SSOutputCmd(SPI_RPI, ENABLE);

	  	// Wlacz SPI1
	SPI_Cmd(SPI_RPI, ENABLE);
	
	
	
	
	
	return 0;
}

//CRC8
uint8_t gencrc8(uint8_t *data, uint16_t len)
{
	uint8_t crc = 0xff;
	uint16_t i, j;
	for (i = 0; i < len; i++) {
		crc ^= data[i];
		for (j = 0; j < 8; j++) {
			if ((crc & 0x80) != 0)
				crc = (uint8_t)((crc << 1) ^ 0x31);
			else
				crc <<= 1;
		}
	}
	return crc;
}

uint8_t Crc8(const void* vptr, int len) {
	const uint8_t *data = (const uint8_t *) vptr;
	unsigned crc = 0;
	int i, j;
	for (j = len; j; j--, data++) {
		crc ^= (*data << 8);
		for (i = 8; i; i--) {
			if (crc & 0x8000)
				crc ^= (0x1070 << 3);
			crc <<= 1;
		}
	}
	return (uint8_t)(crc >> 8);
}
//CRC8/MAXIM
uint8_t CRC8(uint8_t *inData, uint8_t len)
{
	uint8_t crc;
	crc = 0;
	for (; len; len--)
	{
		crc ^= *inData++;
		crc ^= (crc << 3) ^ (crc << 4) ^ (crc << 6);
		crc ^= (crc >> 4) ^ (crc >> 5);
	}
	return crc;

}

extern "C"
{
	void SPI2_IRQHandler(void)
	{
	  /* Store SPI_SLAVE received data */
		if (SPI_I2S_GetFlagStatus(SPI_RPI, SPI_I2S_FLAG_RXNE) != RESET)
		{
			rxData[rxIndex] = SPI_I2S_ReceiveData(SPI_RPI);
			rxIndex++;
			if (rxIndex >= 5)  //5 bytes
			{
				rxIndex = 0;
				txIndex = 0; //TODO it works, but detect end of transmision in other way
				
				//The following instructions are required to clear calculated CRC 
				SPI_Cmd(SPI_RPI, DISABLE);
				SPI_CalculateCRC(SPI_RPI, DISABLE);
				SPI_CalculateCRC(SPI_RPI, ENABLE);
				SPI_Cmd(SPI_RPI, ENABLE);

				
			}
		}

		if (SPI_I2S_GetITStatus(SPI_RPI, SPI_I2S_IT_TXE) != RESET)
		{

			FlagStatus busy = SPI_I2S_GetFlagStatus(SPI_RPI, SPI_I2S_FLAG_BSY);

			if (txIndex < 4)
			{	
				SPI_I2S_SendData(SPI_RPI, txData[txIndex]);
				txIndex++;
			}
			
			if (txIndex == 4)
			{
				SPI_TransmitCRC(SPI_RPI);
				txIndex++;

			} //must be else!! (doesn't send data in next interrupt cycle)
			else if (txIndex >= 5 && busy == RESET)  //5 bytes
			{
				//txIndex = 0; //doesnt work
			}
						
				
		}
		
	}
	
	
	void TIM4_IRQHandler()
	{
		if (TIM_GetITStatus(TIM_ENCODER, TIM_IT_Update) != RESET)
		{
			TIM_ClearITPendingBit(TIM_ENCODER, TIM_IT_Update);
			if (TIM_GetCounter(TIM_ENCODER) > (TIM_ENCODER_OVERFLOW_VALUE/2U))
			{
				encoderRev -= 1;
			}
			else
			{
				encoderRev += 1;
			}

		}
	}
}