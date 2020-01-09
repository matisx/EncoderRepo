#include "i2c_common.h"
#include "i2c_hw.h"



void cI2C_Init(cI2C* i2c)
{
	I2C_Hw_Init(i2c->I2C_Hw);
	
}

uint8_t I2C_Read(tI2C* I2Cx, uint8_t address, uint8_t reg)
{
	uint8_t received_data;
	 
	
	I2C_Hw_Start(I2Cx, address, I2C_TRANSMITTER_MODE, I2C_ACK_DISABLE);
	I2C_Hw_WriteData(I2Cx, reg);
	I2C_Hw_Stop(I2Cx);
	I2C_Hw_Start(I2Cx, address, I2C_RECEIVER_MODE, I2C_ACK_DISABLE);
	received_data = I2C_Hw_ReadNack(I2Cx);
	
	
	
	//I2C_ReadMulti(I2Cx, address, reg, &received_data, 1);
	
	return received_data;
}

uint8_t I2C_ReadMulti(tI2C* I2Cx, uint8_t address, uint8_t reg, uint8_t* data, uint16_t count)
{
	uint8_t ret;
	
	
	//I2C_ACK_ENABLE
	ret = I2C_Hw_Start(I2Cx, address, I2C_TRANSMITTER_MODE, I2C_ACK_ENABLE);
	if (ret != 0)
		return ret;
	if (I2C_Hw_WriteData(I2Cx, reg) != 0)
		return 10;
	
	//mod mx
	//I2C_Hw_Stop(I2Cx);
	
	if (I2C_Hw_Start(I2Cx, address, I2C_RECEIVER_MODE, I2C_ACK_ENABLE) != 0)
		return 11;
				

	while (count--) {
		if (!count) {
			// Last byte 
			//mod mx
			//I2C_Hw_Stop(I2Cx);
			

			*data++ = I2C_Hw_ReadNack(I2Cx);
			
			__enable_irq();

		}
		else {
			if (count == 1)
			{
					__disable_irq();

			}
			*data++ = I2C_Hw_ReadAck(I2Cx);
		}
	}

	/*
	while (count--) {
		if (count==1) {
			// Last byte 
			//mod mx
			//I2C_Hw_Stop(I2Cx);
			
			while ((!(I2Cx->I2C->SR1 & I2C_SR1_BTF)))
				;
			I2Cx->I2C->CR1 &= ~I2C_CR1_ACK;
			
			__disable_irq();
			
			
			I2Cx->I2C->CR1 |= I2C_CR1_STOP;

			*data++  = I2Cx->I2C->DR;
			
			__enable_irq();

			while (!I2C_CheckEvent(I2Cx->I2C, I2C_EVENT_MASTER_BYTE_RECEIVED))
				;
			
			*data++  = I2Cx->I2C->DR;
			
			while ((I2Cx->I2C->CR1 & I2C_CR1_STOP) == 1)
				;

			I2Cx->I2C->CR1 |= I2C_CR1_ACK;
			
			
		}
		else {
			*data++ = I2C_Hw_ReadAck(I2Cx);
		}
	}*/
	
	return 0;

}

uint8_t I2C_ReadMultiNew(tI2C* I2Cx, uint8_t address, uint8_t reg, uint8_t* data, uint16_t count)
{
	uint8_t ret;
	
	// ENTR_CRT_SECTION();

    /* While the bus is busy */
	while (I2C_GetFlagStatus(I2Cx->I2C, I2C_FLAG_BUSY))
		;

	    /* Send START condition */
	I2C_GenerateSTART(I2Cx->I2C, ENABLE);

	    /* Test on EV5 and clear it */
	while (!I2C_CheckEvent(I2Cx->I2C, I2C_EVENT_MASTER_MODE_SELECT))
		;

	    /* Send MPU6050 address for write */
	I2C_Send7bitAddress(I2Cx->I2C, address, I2C_Direction_Transmitter);

	    /* Test on EV6 and clear it */
	while (!I2C_CheckEvent(I2Cx->I2C, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
		;

	    /* Clear EV6 by setting again the PE bit */
	I2C_Cmd(I2Cx->I2C, ENABLE);

	    /* Send the MPU6050's internal address to write to */
	I2C_SendData(I2Cx->I2C, reg);

	    /* Test on EV8 and clear it */
	while (!I2C_CheckEvent(I2Cx->I2C, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
		;

	    /* Send STRAT condition a second time */
	I2C_GenerateSTART(I2Cx->I2C, ENABLE);

	    /* Test on EV5 and clear it */
	while (!I2C_CheckEvent(I2Cx->I2C, I2C_EVENT_MASTER_MODE_SELECT))
		;

	    /* Send MPU6050 address for read */
	I2C_Send7bitAddress(I2Cx->I2C, address, I2C_Direction_Receiver);

	    /* Test on EV6 and clear it */
	while (!I2C_CheckEvent(I2Cx->I2C, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED))
		;

	    /* While there is data to be read */
	while (count)
	{
		if (count == 1)
		{
		    /* Disable Acknowledgement */
			I2C_AcknowledgeConfig(I2Cx->I2C, DISABLE);

			            /* Send STOP Condition */
			I2C_GenerateSTOP(I2Cx->I2C, ENABLE);
		}

		        /* Test on EV7 and clear it */
		if (I2C_CheckEvent(I2Cx->I2C, I2C_EVENT_MASTER_BYTE_RECEIVED))
		{
		    /* Read a byte from the MPU6050 */
			*data = I2C_ReceiveData(I2Cx->I2C);

			            /* Point to the next location where the byte read will be saved */
			data++;

			            /* Decrement the read bytes counter */
			count--;
		}
	}

	    /* Enable Acknowledgement to be ready for another reception */
	I2C_AcknowledgeConfig(I2Cx->I2C, ENABLE);
	// EXT_CRT_SECTION();
		
	
	return 0;
}

uint8_t I2C_ReadNoRegister(tI2C* I2Cx, uint8_t address)
{
	uint8_t data;
	I2C_Hw_Start(I2Cx, address, I2C_RECEIVER_MODE, I2C_ACK_ENABLE);
	/* Also stop condition happens */
	data = I2C_Hw_ReadNack(I2Cx);
	return data;
}

uint8_t I2C_ReadMultiNoRegister(tI2C* I2Cx, uint8_t address, uint8_t* data, uint16_t count)
{
	if (I2C_Hw_Start(I2Cx, address, I2C_RECEIVER_MODE, I2C_ACK_ENABLE) != 0)
	{
		return 1;
	}
	while (count--) {
		if (!count) {
			/* Last byte */
			*data = I2C_Hw_ReadNack(I2Cx);
		}
		else {
			*data = I2C_Hw_ReadAck(I2Cx);
		}
	}
	
	return 0;
}

void I2C_Write(tI2C* I2Cx, uint8_t address, uint8_t reg, uint8_t data) 
{
	
	I2C_Hw_Start(I2Cx, address, I2C_TRANSMITTER_MODE, I2C_ACK_DISABLE);
	I2C_Hw_WriteData(I2Cx, reg);
	I2C_Hw_WriteData(I2Cx, data);
	I2C_Hw_Stop(I2Cx);
	
}
void I2C_WriteNew(tI2C* I2Cx, uint8_t address, uint8_t reg, uint8_t data) 
{

	 // ENTR_CRT_SECTION();

    /* Send START condition */
	I2C_GenerateSTART(I2Cx->I2C, ENABLE);

	    /* Test on EV5 and clear it */
	while (!I2C_CheckEvent(I2Cx->I2C, I2C_EVENT_MASTER_MODE_SELECT))
		;

	    /* Send MPU6050 address for write */
	I2C_Send7bitAddress(I2Cx->I2C, address, I2C_Direction_Transmitter);

	    /* Test on EV6 and clear it */
	while (!I2C_CheckEvent(I2Cx->I2C, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
		;

	    /* Send the MPU6050's internal address to write to */
	I2C_SendData(I2Cx->I2C, reg);

	    /* Test on EV8 and clear it */
	while (!I2C_CheckEvent(I2Cx->I2C, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
		;

	    /* Send the byte to be written */
	I2C_SendData(I2Cx->I2C, data);

	    /* Test on EV8 and clear it */
	while (!I2C_CheckEvent(I2Cx->I2C, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
		;

	    /* Send STOP condition */
	I2C_GenerateSTOP(I2Cx->I2C, ENABLE);

	    // EXT_CRT_SECTION();
}

void I2C_WriteMulti(tI2C* I2Cx, uint8_t address, uint8_t reg, uint8_t* data, uint16_t count) 
{
	I2C_Hw_Start(I2Cx, address, I2C_TRANSMITTER_MODE, I2C_ACK_DISABLE);
	I2C_Hw_WriteData(I2Cx, reg);
	while (count--) {
		I2C_Hw_WriteData(I2Cx, *data++);
	}
	I2C_Hw_Stop(I2Cx);
}

void I2C_WriteNoRegister(tI2C* I2Cx, uint8_t address, uint8_t data)
{
	I2C_Hw_Start(I2Cx, address, I2C_TRANSMITTER_MODE, I2C_ACK_DISABLE);
	I2C_Hw_WriteData(I2Cx, data);
	I2C_Hw_Stop(I2Cx);
}

void I2C_WriteMultiNoRegister(tI2C* I2Cx, uint8_t address, uint8_t* data, uint16_t count)
{
	I2C_Hw_Start(I2Cx, address, I2C_TRANSMITTER_MODE, I2C_ACK_DISABLE);
	while (count--) {
		I2C_Hw_WriteData(I2Cx, *data++);
	}
	I2C_Hw_Stop(I2Cx);
}


//return 1 if device will send ACK
uint8_t I2C_IsDeviceConnected(tI2C* I2Cx, uint8_t address)
{
	uint8_t connected = 0;
	/* Try to start, function will return 0 in case device will send ACK */
	if (!I2C_Hw_Start(I2Cx, address, I2C_TRANSMITTER_MODE, I2C_ACK_ENABLE)) {
		connected = 1;
	}
	
	/* STOP I2C */
	I2C_Hw_Stop(I2Cx);
	
	/* Return status */
	return connected;
}
