#include "i2c_hw.h"

#define I2C_TIMEOUT_HIGH 30000 //20000
#define I2C_TIMEOUT_LOW	 500 //mx

static uint32_t I2C_MaxTimeout = I2C_TIMEOUT_HIGH;

static uint32_t I2C_Timeout;
//static uint32_t I2C_INT_Clocks[3] = { 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF };

void I2C_SetLowTimeout()
{
	I2C_MaxTimeout = I2C_TIMEOUT_LOW;
}
void I2C_SetHighTimeout()
{
	I2C_MaxTimeout = I2C_TIMEOUT_HIGH;
}


void I2C_Hw_Init(tI2C* hw)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	I2C_InitTypeDef I2C_InitStruct;

	GPIO_InitStructure.GPIO_Pin = hw->GPIO_SCL.GPIO.GPIO_Pin;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
	
	GPIO_Init(hw->GPIO_SCL.GPIO.GPIOx, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = hw->GPIO_SDA.GPIO.GPIO_Pin;
	GPIO_Init(hw->GPIO_SDA.GPIO.GPIOx, &GPIO_InitStructure);
	

	
	if(hw->GPIO_SCL.GPIO_AF != 0)
		GPIO_PinRemapConfig(hw->GPIO_SCL.GPIO_AF, ENABLE); //GPIO_Mode_AF_OD jest wystarczaj¹ce gdy i2c jest domyœln¹ AF
	if (hw->GPIO_SDA.GPIO_AF != 0)
		GPIO_PinRemapConfig(hw->GPIO_SDA.GPIO_AF, ENABLE);
	
	//I2C_SoftwareResetCmd(hw->I2C, ENABLE);
	
	I2C_Cmd(hw->I2C, DISABLE);
	
	I2C_DeInit(hw->I2C);

	I2C_InitStruct.I2C_ClockSpeed = hw->clock;
	I2C_InitStruct.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	I2C_InitStruct.I2C_Mode = I2C_Mode_I2C;
	I2C_InitStruct.I2C_OwnAddress1 = hw->address;
	I2C_InitStruct.I2C_Ack = I2C_Ack_Enable;//
	I2C_InitStruct.I2C_DutyCycle = I2C_DutyCycle_2;
	I2C_Init(hw->I2C, &I2C_InitStruct);
	
	I2C_Cmd(hw->I2C, ENABLE);

}

// Hardware functions
uint8_t I2C_Hw_Start(tI2C* I2Cx, uint8_t address, uint8_t direction, uint8_t ack) 
{
	unsigned int dummy;
	//while (I2C_GetFlagStatus(I2Cx->I2C, I2C_FLAG_BUSY))
	//	;
	
	// Generate I2C start pulse 
	I2Cx->I2C->CR1 |= I2C_CR1_START;
	
	I2C_Timeout = 0;
	//Wait till I2C is busy 
	I2C_Timeout =  I2C_MaxTimeout;
	while (!(I2Cx->I2C->SR1 & I2C_SR1_SB)) {
		if (--I2C_Timeout == 0x00) {
			
			return 1;
		}
	}
	dummy = I2Cx->I2C->SR1;
	dummy = I2Cx->I2C->SR2;

	// Enable ack if we select it 
	if (ack) {
		I2Cx->I2C->CR1 |= I2C_CR1_ACK;
	}

	// Send write/read bit 
	if (direction == I2C_TRANSMITTER_MODE) {
		// Send address with zero last bit 
		I2Cx->I2C->DR = address & ~I2C_OAR1_ADD0;
		
		// Wait till finished 
		I2C_Timeout = I2C_MaxTimeout;
		while (!(I2Cx->I2C->SR1 & I2C_SR1_ADDR)) {
			if (--I2C_Timeout == 0x00) {
				return 2;
			}
		}
	}
	
	
	if (direction == I2C_RECEIVER_MODE) {
		// Send address with 1 last bit 
		I2Cx->I2C->DR = address | I2C_OAR1_ADD0;
		
		// Wait till finished 
		I2C_Timeout = I2C_MaxTimeout;
		while (!I2C_CheckEvent(I2Cx->I2C, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED)) {
			if (--I2C_Timeout == 0x00) {
				return 3;
			}
		}
	}
	
	// Read status register to clear ADDR flag 
	dummy = I2Cx->I2C->SR1;
	dummy = I2Cx->I2C->SR2;
	
	// Return 0, everything ok
	return 0;
		
}
 


uint8_t I2C_Hw_WriteData(tI2C* I2Cx, uint8_t data)
{
	/* Wait till I2C is not busy anymore */
	I2C_Timeout = I2C_MaxTimeout;
	while (!(I2Cx->I2C->SR1 & I2C_SR1_TXE) && I2C_Timeout) {
		if (--I2C_Timeout == 0x00)
			return 1;
	}
	
	/* Send I2C data */
	I2Cx->I2C->DR = data;
	return 0;
}

uint8_t I2C_Hw_ReadAck(tI2C* I2Cx) 
{
	uint8_t data;
	
	/* Enable ACK */
	I2Cx->I2C->CR1 |= I2C_CR1_ACK;
	
	/* Wait till not received */
	I2C_Timeout = I2C_MaxTimeout;
	while (!I2C_CheckEvent(I2Cx->I2C, I2C_EVENT_MASTER_BYTE_RECEIVED)) {
		if (--I2C_Timeout == 0x00) {
			return 1;
		}
	}
	
	/* Read data */
	data = I2Cx->I2C->DR;
	
	/* Return data */
	return data;
}

uint8_t I2C_Hw_ReadNack(tI2C* I2Cx)
{
	uint8_t data;
	
	/* Disable ACK */
	I2Cx->I2C->CR1 &= ~I2C_CR1_ACK;
	
	/* Generate stop */
	I2Cx->I2C->CR1 |= I2C_CR1_STOP;
	
	/* Wait till received */
	I2C_Timeout = I2C_MaxTimeout;
	while (!I2C_CheckEvent(I2Cx->I2C, I2C_EVENT_MASTER_BYTE_RECEIVED)) {
		if (--I2C_Timeout == 0x00) {
			return 1;
		}
	}

	/* Read data */
	data = I2Cx->I2C->DR;
	
	/* Return data */
	return data;
}

uint8_t I2C_Hw_Stop(tI2C* I2Cx)
{
	/* Wait till transmitter not empty */
	I2C_Timeout = I2C_MaxTimeout;
	while (((!(I2Cx->I2C->SR1 & I2C_SR1_TXE)) || (!(I2Cx->I2C->SR1 & I2C_SR1_BTF)))) {
		if (--I2C_Timeout == 0x00) {
			/* Generate stop */
			I2Cx->I2C->CR1 |= I2C_CR1_STOP;
			return 1;
		}
	}
	
	/* Generate stop */
	I2Cx->I2C->CR1 |= I2C_CR1_STOP;
	
	/* Return 0, everything ok */
	return 0;
}

