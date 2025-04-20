#include "stm32f407xx_i2c_driver.h"




static void I2C_GenerateStartCondition(I2C_RegDef_t *pI2Cx);
static void I2C_ExecuteAddressPhaseWrite(I2C_RegDef_t *pI2Cx, uint8_t SlaveAddr);
static void I2C_ExecuteAddressPhaseRead(I2C_RegDef_t *pI2Cx, uint8_t SlaveAddr);
static void I2C_ClearADDRFlag(I2C_Handle_t *pI2CHandle);
static void I2C_MasterHandleRXNEInterrupt(I2C_Handle_t *pI2CHandle);
static void I2C_MasterHandleTXEInterrupt(I2C_Handle_t *pI2CHandle);

static void I2C_GenerateStartCondition(I2C_RegDef_t *pI2Cx)
{
	pI2Cx->CR1 |= (1<<I2C_CR1_START);
}

static void I2C_ExecuteAddressPhaseWrite(I2C_RegDef_t *pI2Cx, uint8_t SlaveAddr)
{
	SlaveAddr = SlaveAddr << 1;
	SlaveAddr &= ~(1);
	pI2Cx->DR = SlaveAddr;
}

static void I2C_ExecuteAddressPhaseRead(I2C_RegDef_t *pI2Cx, uint8_t SlaveAddr)
{
	SlaveAddr = SlaveAddr << 1;
	SlaveAddr |= 1;
	pI2Cx->DR = SlaveAddr;
}

static void I2C_ClearADDRFlag(I2C_Handle_t *pI2CHandle)
{
	// uint32_t dummyRead = pI2Cx->SR1;
	// dummyRead = pI2Cx->SR2;
	// (void)dummyRead;
	uint32_t dummyRead;
	//check for device mode
	if(pI2CHandle->pI2Cx->SR2 & (1 << I2C_SR2_MSL))
	{
		//device is in master mode
		if(pI2CHandle->TxRxState == I2C_BUSY_IN_RX)
		{
			if(pI2CHandle->RxSize == 1)
			{
				//first disable the ack
				I2C_ManageAcking(pI2CHandle->pI2Cx, DISABLE);

				//clear ADDR flag
				dummyRead = pI2CHandle->pI2Cx->SR1;
				dummyRead = pI2CHandle->pI2Cx->SR2;
				(void)dummyRead;
			}
		}
		else
		{
			//clear ADDR flag
			dummyRead = pI2CHandle->pI2Cx->SR1;
			dummyRead = pI2CHandle->pI2Cx->SR2;
			(void)dummyRead;
		}
	}
	else
	{
		//clear ADDR flag
		dummyRead = pI2CHandle->pI2Cx->SR1;
		dummyRead = pI2CHandle->pI2Cx->SR2;
		(void)dummyRead;
	}
}

void I2C_GenerateStopCondition(I2C_RegDef_t *pI2Cx)
{
	pI2Cx->CR1 |= (1<<I2C_CR1_STOP);
}

void I2C_SlaveEnableDisableCallbackEvents(I2C_RegDef_t *pI2Cx, uint8_t EnOrDi)
{
	if(EnOrDi == ENABLE)
	{
		pI2Cx->CR2 |=  (1 << I2C_CR2_ITBUFEN);
		pI2Cx->CR2 |=  (1 << I2C_CR2_ITEVTEN);
		pI2Cx->CR2 |=  (1 << I2C_CR2_ITERREN);

	}
	else
	{
		pI2Cx->CR2 &=  ~(1 << I2C_CR2_ITBUFEN);
		pI2Cx->CR2 &=  ~(1 << I2C_CR2_ITEVTEN);
		pI2Cx->CR2 &=  ~(1 << I2C_CR2_ITERREN);

	}

}

void I2C_PeripheralControl(I2C_RegDef_t *pI2Cx, uint8_t EnOrDi)
{
	if(EnOrDi == ENABLE)
	{
		pI2Cx->CR1 |=  (1 << I2C_CR1_PE);
	}else
	{
		pI2Cx->CR1 &=  ~(1 << I2C_CR1_PE);
	}
}

void I2C_Init(I2C_Handle_t *pI2CHandle)
{
	uint32_t tempreg = 0;


	//enable clock for i2c peripheral

	I2C_PeriClockControl(pI2CHandle->pI2Cx, ENABLE);
	
	//ack control bit
	tempreg |= pI2CHandle->I2C_Config.I2C_AckControl << 10;
	pI2CHandle->pI2Cx->CR1 = tempreg;

	//configure the FREQ field of CR2
	tempreg = 0;
	tempreg |= RCC_GetPCLK1Value()/1000000U;
	pI2CHandle->pI2Cx->CR2 = tempreg & (0X3F);

	//program the device own address
	tempreg = 0;
	tempreg |= pI2CHandle->I2C_Config.I2C_DeviceAddress << 1;
	tempreg |= (1<<14);
	pI2CHandle->pI2Cx->OAR1 = tempreg;

	//CCR calculation clock control register
	uint16_t ccr_value = 0;
	tempreg = 0;
	if(pI2CHandle->I2C_Config.I2C_SCLSpeed <= I2C_SCL_SPEED_SM)
	{
		//mode is standard mode
		ccr_value = (RCC_GetPCLK1Value() / (2 * pI2CHandle->I2C_Config.I2C_SCLSpeed));
		tempreg |= (ccr_value & 0xFFF);
	}
	else
	{
		//mode is fast mode
		tempreg |= (1<<15);
		tempreg |= (pI2CHandle->I2C_Config.I2C_FMDutyCycle << 14);
		if(pI2CHandle->I2C_Config.I2C_FMDutyCycle == I2C_FM_DUTY_2)
		{
			ccr_value = (RCC_GetPCLK1Value() / (3 * pI2CHandle->I2C_Config.I2C_SCLSpeed));
		}
		else
		{
			ccr_value = (RCC_GetPCLK1Value() / (25 * pI2CHandle->I2C_Config.I2C_SCLSpeed));
		}
	}
	pI2CHandle->pI2Cx->CCR = tempreg;

	//TRISE configuration Fpclk1 * Trise(max) + 1
	if(pI2CHandle->I2C_Config.I2C_SCLSpeed <= I2C_SCL_SPEED_SM)
	{
		//mode is standard mode
		tempreg = (RCC_GetPCLK1Value()/1000000U) + 1;
	}
	else
	{
		//mode is fast mode
		tempreg = (RCC_GetPCLK1Value() * 300/1000000U) + 1;
	}
	pI2CHandle->pI2Cx->TRISE = tempreg;
}

uint8_t I2C_GetFlagStatus(I2C_RegDef_t *pI2Cx, uint32_t FlagName)
{
	if(pI2Cx->SR1 & FlagName)
	{
		return FLAG_SET;
	}
	return FLAG_RESET;
}

void I2C_MasterSendData(I2C_Handle_t *pI2CHandle ,uint8_t *pTxBuffer, uint32_t Len, uint8_t SlaveAddr,uint8_t Sr,I2C_Handle_t *pI2CHandle2)
{
	//reference manual 27.3.3 figure 243
	//1. generate the start condition
	I2C_GenerateStartCondition(pI2CHandle->pI2Cx);

	//2. confirm that start generation is completed by checking the SB flag in SR1
	// Note: until SB is cleared SCL will be stretched(pulled to low)
	//By reference manual: Cleared by software by reading SR1 followed by writing DR, or by hardware when PE = 0
	while(!I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_FLAG_SB));

	//3. Send the address of slave with r/nw bit set to w(0) (total 8 bits)
	I2C_ExecuteAddressPhaseWrite(pI2CHandle->pI2Cx, SlaveAddr);

	//4. Confirm that address phase is completed by checking ADDR flag in the SR1
	while(!I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_FLAG_ADDR));

	//5. clear the ADDR flag according to its software sequence
	//Note: Until ADDR is cleared SCL will be stretched(pulled to low)
	I2C_ClearADDRFlag(pI2CHandle);
	if(pI2CHandle2)	I2C_ClearADDRFlag(pI2CHandle2);

	//6.Send the data until len becomes 0
	for(int i = Len; i>0; i--)
	{
		while(!I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_FLAG_TXE)); //Wait till TXE is set

		pI2CHandle->pI2Cx->DR = *pTxBuffer;
		pTxBuffer++;
		if(pI2CHandle2)
		{
			while(!I2C_GetFlagStatus(pI2CHandle2->pI2Cx, I2C_FLAG_RXNE)); //Wait till RXNE is set
			int dummy_read = pI2CHandle2->pI2Cx->DR; //DR is read before next byte is received
			(void)dummy_read; //avoid unused warning			
		}
	} 
	
	//7. when Len becomes zero wait for TXE = 1 and BTF = 1 before generating the STOP condition
	// Note: TXE = 1, BTF = 1, means that both SR and DR are empty and next transmission should begin
	// when BTF = 1 SCL will be stretched(pulled to low)
	while(!I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_FLAG_TXE));

	while(!I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_FLAG_BTF));

	//8. Generate STOP condition and master need not to wait for the completion of STOP condition
	// Note: generating STOP, automatically clears the BTF
	if(Sr == I2C_DISABLE_SR )
		I2C_GenerateStopCondition(pI2CHandle->pI2Cx);
}

void I2C_MasterReceiveData(I2C_Handle_t *pI2CHandle ,uint8_t *pRxBuffer, uint32_t Len, uint8_t SlaveAddr,uint8_t Sr)
{
	//reference manual 27.3.3 figure 243
	//1. generate the start condition
	I2C_GenerateStartCondition(pI2CHandle->pI2Cx);

	//2. confirm that start generation is completed by checking the SB flag in SR1
	// Note: until SB is cleared SCL will be stretched(pulled to low)
	//By reference manual: Cleared by software by reading SR1 followed by writing DR, or by hardware when PE = 0
	while(!I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_FLAG_SB));

	//3. Send the address of slave with r/nw bit set to R(1) (total 8 bits)
	I2C_ExecuteAddressPhaseRead(pI2CHandle->pI2Cx, SlaveAddr);

	//4. Confirm that address phase is completed by checking ADDR flag in the SR1
	while(!I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_FLAG_ADDR));


	//procedure to read only 1 byte from slave

	if(Len == 1)
	{
		//Disable Acking
		I2C_ManageAcking(pI2CHandle->pI2Cx, I2C_ACK_DISABLE);

		//Clear the ADDR flag according to its software sequence
		//Note: Until ADDR is cleared SCL will be stretched(pulled to low)
		I2C_ClearADDRFlag(pI2CHandle);

		//wait until RXNE becomes 1
		while(!I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_FLAG_RXNE));

		//generate STOP condition
		I2C_GenerateStopCondition(pI2CHandle->pI2Cx);		

		//read data in to buffer	
		*pRxBuffer = pI2CHandle->pI2Cx->DR;
	}

	//procedure to read only 1 byte from slave when len > 1
	if(Len > 1)
	{

		//Clear the ADDR flag according to its software sequence
		//Note: Until ADDR is cleared SCL will be stretched(pulled to low)
		I2C_ClearADDRFlag(pI2CHandle);

		//read data until len becomes 0
		for(int i = Len; i>0; i--)
		{
			//wait until RXNE becomes 1
			while(!I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_FLAG_RXNE));

			if(i == 2) //if last 2 bytes are remaining
			{
				//Disable Acking
				
				I2C_ManageAcking(pI2CHandle->pI2Cx,I2C_ACK_DISABLE);
				//generate STOP condition
				I2C_GenerateStopCondition(pI2CHandle->pI2Cx);
			}


			//read data from data register in to buffer
			*pRxBuffer = pI2CHandle->pI2Cx->DR;

			//increment buffer address
			pRxBuffer++;
		}
	}
	//re-enable acking
	if(pI2CHandle->I2C_Config.I2C_AckControl == I2C_ACK_ENABLE)
	{
		I2C_ManageAcking(pI2CHandle->pI2Cx, I2C_ACK_ENABLE);
	}
	
}


/*********************************************************************
 * @fn      		  - I2C_MasterSendDataIT
 *
 * @brief             -
 *
 * @param[in]         -
 * @param[in]         -
 * @param[in]         -
 *
 * @return            -
 *
 * @Note              -  Complete the below code . Also include the function prototype in header file

 */
//just trigger the start condition
uint8_t I2C_MasterSendDataIT(I2C_Handle_t *pI2CHandle,uint8_t *pTxBuffer, uint32_t Len,uint8_t SlaveAddr,uint8_t Sr)
{

	uint8_t busystate = pI2CHandle->TxRxState;

	if( (busystate != I2C_BUSY_IN_TX) && (busystate != I2C_BUSY_IN_RX))
	{
		pI2CHandle->pTxBuffer = pTxBuffer;
		pI2CHandle->TxLen = Len;
		pI2CHandle->TxRxState = I2C_BUSY_IN_TX;
		pI2CHandle->DevAddr = SlaveAddr;
		pI2CHandle->Sr = Sr;

		//Implement code to Generate START Condition
		I2C_GenerateStartCondition(pI2CHandle->pI2Cx);

		//Implement the code to enable ITBUFEN Control Bit
		pI2CHandle->pI2Cx->CR2 |= ( 1 << I2C_CR2_ITBUFEN);

		//Implement the code to enable ITEVFEN Control Bit
		pI2CHandle->pI2Cx->CR2 |= ( 1 << I2C_CR2_ITEVTEN);

		//Implement the code to enable ITERREN Control Bit
		pI2CHandle->pI2Cx->CR2 |= ( 1 << I2C_CR2_ITERREN);


		I2C_SlaveEnableDisableCallbackEvents(I2C2, ENABLE);//add I2C interrupt en/disable here
	}
	return busystate;
}

/*********************************************************************
 * @fn      		  - I2C_MasterReceiveDataIT
 *
 * @brief             -
 *
 * @param[in]         -
 * @param[in]         -
 * @param[in]         -
 *
 * @return            -
 *
 * @Note              - Complete the below code . Also include the fn prototype in header file

 */
uint8_t I2C_MasterReceiveDataIT(I2C_Handle_t *pI2CHandle,uint8_t *pRxBuffer, uint8_t Len, uint8_t SlaveAddr,uint8_t Sr)
{
	uint8_t busystate = pI2CHandle->TxRxState;

	if( (busystate != I2C_BUSY_IN_TX) && (busystate != I2C_BUSY_IN_RX))
	{
		pI2CHandle->pRxBuffer = pRxBuffer;
		pI2CHandle->RxLen = Len;
		pI2CHandle->TxRxState = I2C_BUSY_IN_RX;
		pI2CHandle->RxSize = Len; //Rxsize is used in the ISR code to manage the data reception 
		pI2CHandle->DevAddr = SlaveAddr;
		pI2CHandle->Sr = Sr;

		//Implement code to Generate START Condition
		I2C_GenerateStartCondition(pI2CHandle->pI2Cx);

		//Implement the code to enable ITBUFEN Control Bit
		pI2CHandle->pI2Cx->CR2 |= ( 1 << I2C_CR2_ITBUFEN);

		//Implement the code to enable ITEVFEN Control Bit
		pI2CHandle->pI2Cx->CR2 |= ( 1 << I2C_CR2_ITEVTEN);

		//Implement the code to enable ITERREN Control Bit
		pI2CHandle->pI2Cx->CR2 |= ( 1 << I2C_CR2_ITERREN);
		
	}

	return busystate;
}


void I2C_SlaveSendData(I2C_RegDef_t *pI2Cx, uint8_t data)
{
	pI2Cx->DR = data;
}

uint8_t I2C_SlaveReceiveData(I2C_RegDef_t *pI2Cx)
{
	return (uint8_t)pI2Cx->DR;
}

void I2C_ManageAcking(I2C_RegDef_t *pI2Cx, uint8_t EnOrDi)
{
	if(EnOrDi == I2C_ACK_ENABLE)
	{
		pI2Cx->CR1 |= (1<<I2C_CR1_ACK);
	}
	else
	{
		pI2Cx->CR1 &= ~(1<<I2C_CR1_ACK);
	}
}

void I2C_PeriClockControl(I2C_RegDef_t *pI2Cx, uint8_t EnorDi)
{

	if(EnorDi == ENABLE)
	{
		if(pI2Cx == I2C1)
		{
			I2C1_PCLK_EN();
		}else if (pI2Cx == I2C2)
		{
			I2C2_PCLK_EN();
		}else if (pI2Cx == I2C3)
		{
			I2C3_PCLK_EN();
		}
	}
	else
	{
		if(pI2Cx == I2C1)
		{
			I2C1_PCLK_DI();
		}else if (pI2Cx == I2C2)
		{
			I2C2_PCLK_DI();
		}else if (pI2Cx == I2C3)
		{
			I2C3_PCLK_DI();
		}
	}
}


void I2C_IRQInterruptConfig(uint8_t IRQNumber, uint8_t EnorDi)
{

	if(EnorDi == ENABLE)
	{
		if(IRQNumber <= 31)
		{
			//program ISER0 register
			*NVIC_ISER0 |= ( 1 << IRQNumber );

		}else if(IRQNumber > 31 && IRQNumber < 64 ) //32 to 63
		{
			//program ISER1 register
			*NVIC_ISER1 |= ( 1 << (IRQNumber % 32) );
		}
		else if(IRQNumber >= 64 && IRQNumber < 96 )
		{
			//program ISER3 register //64 to 95
			*NVIC_ISER3 |= ( 1 << (IRQNumber % 64) );
		}
	}else
	{
		if(IRQNumber <= 31)
		{
			//program ICER0 register
			*NVIC_ICER0 |= ( 1 << IRQNumber );
		}else if(IRQNumber > 31 && IRQNumber < 64 )
		{
			//program ICER1 register
			*NVIC_ICER1 |= ( 1 << (IRQNumber % 32) );
		}
		else if(IRQNumber >= 6 && IRQNumber < 96 )
		{
			//program ICER3 register
			*NVIC_ICER3 |= ( 1 << (IRQNumber % 64) );
		}
	}

}

void I2C_IRQPriorityConfig(uint8_t IRQNumber, uint32_t IRQPriority)
{
	//1. first lets find out the ipr register
	uint8_t iprx = IRQNumber / 4;
	uint8_t iprx_section  = IRQNumber %4 ;

	uint8_t shift_amount = ( 8 * iprx_section) + ( 8 - NO_PR_BITS_IMPLEMENTED) ;

	*(  NVIC_PR_BASE_ADDR + iprx ) |=  ( IRQPriority << shift_amount );

}

static void I2C_MasterHandleTXEInterrupt(I2C_Handle_t *pI2CHandle)
{

	if(pI2CHandle->TxLen > 0)
	{
		// printf("txe\n");
		//1. load the data in to DR
		pI2CHandle->pI2Cx->DR = *(pI2CHandle->pTxBuffer);

		//2. decrement the TxLen
		pI2CHandle->TxLen--;

		//3. Increment the buffer address
		pI2CHandle->pTxBuffer++;

		// while(!I2C_GetFlagStatus(pI2CHandle2->pI2Cx, I2C_FLAG_RXNE)); //Wait till RXNE is set
		// int dummy_read = pI2CHandle2->pI2Cx->DR; //DR is read before next byte is received
		// (void)dummy_read; //avoid unused warning
	}

}

static void I2C_MasterHandleRXNEInterrupt(I2C_Handle_t *pI2CHandle )
{
	// printf("rxne it\n");
	//We have to do the data reception
	if(pI2CHandle->RxSize == 1)
	{
		*pI2CHandle->pRxBuffer = pI2CHandle->pI2Cx->DR;
		pI2CHandle->RxLen--;

	}


	if(pI2CHandle->RxSize > 1)
	{
		if(pI2CHandle->RxLen == 2)
		{
			//clear the ack bit
			I2C_ManageAcking(pI2CHandle->pI2Cx,DISABLE);
		}

			//read DR
			*pI2CHandle->pRxBuffer = pI2CHandle->pI2Cx->DR;
			pI2CHandle->pRxBuffer++;
			pI2CHandle->RxLen--;
	}

	if(pI2CHandle->RxLen == 0 )
	{
		//close the I2C data reception and notify the application

		//1. generate the stop condition
		if(pI2CHandle->Sr == I2C_DISABLE_SR)
			I2C_GenerateStopCondition(pI2CHandle->pI2Cx);

		//2 . Close the I2C rx
		I2C_CloseReceiveData(pI2CHandle);

		//3. Notify the application
		I2C_ApplicationEventCallback(pI2CHandle,I2C_EV_RX_CMPLT);
	}
}


void I2C_EV_IRQHandling(I2C_Handle_t *pI2CHandle)
{
	//Interrupt handling for master and slave mode of a device
	uint32_t temp1, temp2, temp3;

	temp1 = pI2CHandle->pI2Cx->CR2 & (1 << I2C_CR2_ITEVTEN);
	temp2 = pI2CHandle->pI2Cx->CR2 & (1 << I2C_CR2_ITBUFEN);
	temp3 = pI2CHandle->pI2Cx->SR1 & (1 << I2C_SR1_SB);

	//1. Handle for interrupt generated by SB event
	// Note: SB flag is only applicable in master mode
	if(temp1 && temp3)
	{
		//printf("sb\n");
		//SB flag is set
		//the interrupt is generated because of SB event
		//the block will not executed in slave mode because for slave SB is always zero
		//in this block lets executed the address phase
		if(pI2CHandle->TxRxState == I2C_BUSY_IN_TX)
		{
			I2C_ExecuteAddressPhaseWrite(pI2CHandle->pI2Cx, pI2CHandle->DevAddr);
		}
		else if(pI2CHandle->TxRxState == I2C_BUSY_IN_RX)
		{
			I2C_ExecuteAddressPhaseRead(pI2CHandle->pI2Cx, pI2CHandle->DevAddr);
		}
	}


	//2. handle for interrupt generated by ADDR event
	//Note: when master mode: address is sent
	//Note: when slave mode: address matched with own address
	temp3 = pI2CHandle->pI2Cx->SR1 & (1 << I2C_SR1_ADDR);
	if(temp1 && temp3)
	{
		//printf("addr\n");
		//ADDR flag is set
		//the interrupt is generated because of ADDR event
		I2C_ClearADDRFlag(pI2CHandle);
	}


	//3. handle for interrupt generated by BTF(Byte Transfer Finished) event
	temp3 = pI2CHandle->pI2Cx->SR1 & (1 << I2C_SR1_BTF);
	if(temp1 && temp3)
	{
		//BTF flag is set
		if(pI2CHandle->TxRxState == I2C_BUSY_IN_TX)
		{
			//BTF, TXE = 1
			if(pI2CHandle->TxLen == 0)
			{
				//printf("len=0\n");
				//1. generate STOP condition
				if(pI2CHandle->Sr == I2C_DISABLE_SR)
					I2C_GenerateStopCondition(pI2CHandle->pI2Cx);

				//2. reset all the	member elements of the handle structure
				I2C_CloseSendData(pI2CHandle);

				//3. notify the application about transmission complete	
				I2C_ApplicationEventCallback(pI2CHandle, I2C_EV_TX_CMPLT);			
			}



		}
		else if(pI2CHandle->TxRxState == I2C_BUSY_IN_RX)
		{	
			;
		}
	}


	//4. Handle for  interrupt generated by STOPF event
	// Note: Stop detection flag(STOPF) is applicable only slave mode. For master this flag will never be set
	// The below code block will not executed by the master since STOPF will not set in master mode
	temp3 = pI2CHandle->pI2Cx->SR1 & (1 << I2C_SR1_STOPF);
	if(temp1 && temp3)
	{
		//STOP flag is set
		//Clear the STOPF (i.e. 1)read SR1 2) Write to CR1)

		pI2CHandle->pI2Cx->CR1 |= 0x0000;

		//notify the application about transmission complete	
		I2C_ApplicationEventCallback(pI2CHandle, I2C_EV_STOP);	
	}


	//Handle for interrupt generated by TXE event
	temp3 = pI2CHandle->pI2Cx->SR1 & (1 << I2C_SR1_TXE);
	if(temp1 && temp2 && temp3)
	{
		if(pI2CHandle->pI2Cx->SR2 & (1 << I2C_SR2_MSL))
		{
			//TXE flag is set
			//we have to do the data transmission
			if(pI2CHandle->TxRxState == I2C_BUSY_IN_TX)
			{
				// printf("m t\n");
				//BTF, TXE = 1
				I2C_MasterHandleTXEInterrupt(pI2CHandle);
			}			
		}
		else
		{
			//slave
			// printf("slave\n");
			//make sure that slave is really in transmitter mode
			if(pI2CHandle->pI2Cx->SR2 & (1<<I2C_SR2_TRA))
			{
				// printf("s t\n");
				I2C_ApplicationEventCallback(pI2CHandle, I2C_EV_DATA_REQ);
			}
		}

	}



	//Handle for interrupt generated by RXNE event
	temp3 = pI2CHandle->pI2Cx->SR1 & (1 << I2C_SR1_RXNE);
	if(temp1 && temp2 && temp3)
	{
		//RXNE flag is set
		if(pI2CHandle->pI2Cx->SR2 & (1 << I2C_SR2_MSL))
		{
			// printf("rxne %x\n",pI2CHandle->I2C_Config.I2C_DeviceAddress);
			if(pI2CHandle->TxRxState == I2C_BUSY_IN_RX)
			{
				// printf("m r\n");
				//we have to do the data reception
				I2C_MasterHandleRXNEInterrupt(pI2CHandle);			
			}
		}
		else
		{
			// printf("s r\n");
			I2C_ApplicationEventCallback(pI2CHandle,I2C_EV_RX_CMPLT);
			// printf("2\n");
			// int rcv_data = I2C_SlaveReceiveData(pI2CHandle->pI2Cx);
			// (void)rcv_data;
			// printf("%d\n",rcv_data);
		}
	
	}
	else
	{
		//slave
		//make sure that slave is really in transmitter mode
		if((pI2CHandle->pI2Cx->SR2 & (1<<I2C_SR2_TRA)) == 0)
		{
			I2C_ApplicationEventCallback(pI2CHandle, I2C_EV_DATA_RCV);

		}
	}
#if 0
	if(temp1 && temp2 && temp3)
	{
		//check device mode .
		if(pI2CHandle->pI2Cx->SR2 & ( 1 << I2C_SR2_MSL))
		{
			//The device is master

			//RXNE flag is set
			if(pI2CHandle->TxRxState == I2C_BUSY_IN_RX)
			{
				I2C_MasterHandleRXNEInterrupt(pI2CHandle);

			}

		}else
		{
			//slave
			//make sure that the slave is really in receiver mode
			if(!(pI2CHandle->pI2Cx->SR2 & ( 1 << I2C_SR2_TRA)))
			{
				I2C_ApplicationEventCallback(pI2CHandle,I2C_EV_DATA_RCV);
			}
		}
	}
#endif
}

void I2C_CloseReceiveData(I2C_Handle_t *pI2CHandle)
{
	pI2CHandle->pI2Cx->CR2 &= ~(1 << I2C_CR2_ITBUFEN);
	pI2CHandle->pI2Cx->CR2 &= ~(1 << I2C_CR2_ITEVTEN);


	pI2CHandle->TxRxState = I2C_READY;
	pI2CHandle->pRxBuffer = NULL;
	pI2CHandle->RxLen = 0;
	pI2CHandle->RxSize = 0;

	if(pI2CHandle->I2C_Config.I2C_AckControl == I2C_ACK_ENABLE)
	{
		I2C_ManageAcking(pI2CHandle->pI2Cx, ENABLE);
	}
		
}
void I2C_CloseSendData(I2C_Handle_t *pI2CHandle)
{
	pI2CHandle->pI2Cx->CR2 &= ~(1 << I2C_CR2_ITBUFEN);
	pI2CHandle->pI2Cx->CR2 &= ~(1 << I2C_CR2_ITEVTEN);


	pI2CHandle->TxRxState = I2C_READY;
	pI2CHandle->pTxBuffer = NULL;
	pI2CHandle->TxLen = 0;
}


void I2C_ER_IRQHandling(I2C_Handle_t *pI2CHandle)
{

	uint32_t temp1,temp2;

    //Know the status of  ITERREN control bit in the CR2
	temp2 = (pI2CHandle->pI2Cx->CR2) & ( 1 << I2C_CR2_ITERREN);


/***********************Check for Bus error************************************/ 
	temp1 = (pI2CHandle->pI2Cx->SR1) & ( 1<< I2C_SR1_BERR);
	if(temp1  && temp2 )
	{
		//This is Bus error
		
		//Implement the code to clear the buss error flag 
		pI2CHandle->pI2Cx->SR1 &= ~( 1 << I2C_SR1_BERR);
		
		//Implement the code to notify the application about the error 
	   I2C_ApplicationEventCallback(pI2CHandle,I2C_ERROR_BERR);
	}

/***********************Check for arbitration lost error************************************/ 
	temp1 = (pI2CHandle->pI2Cx->SR1) & ( 1 << I2C_SR1_ARLO );
	if(temp1  && temp2)
	{
		//This is arbitration lost error
		
		//Implement the code to clear the arbitration lost error flag
		pI2CHandle->pI2Cx->SR1 &= ~( 1 << I2C_SR1_ARLO);

		//Implement the code to notify the application about the error 
		I2C_ApplicationEventCallback(pI2CHandle,I2C_ERROR_ARLO);
	   
	}

/***********************Check for ACK failure  error************************************/ 

	temp1 = (pI2CHandle->pI2Cx->SR1) & ( 1 << I2C_SR1_AF);
	if(temp1  && temp2)
	{
		//This is ACK failure error
		
	    //Implement the code to clear the ACK failure error flag
		pI2CHandle->pI2Cx->SR1 &= ~( 1 << I2C_SR1_AF);

		//Implement the code to notify the application about the error 
		I2C_ApplicationEventCallback(pI2CHandle,I2C_ERROR_AF);
	}

/***********************Check for Overrun/underrun error************************************/
	temp1 = (pI2CHandle->pI2Cx->SR1) & ( 1 << I2C_SR1_OVR);
	if(temp1  && temp2)
	{
		//This is Overrun/underrun
		
	    //Implement the code to clear the Overrun/underrun error flag
		pI2CHandle->pI2Cx->SR1 &= ~( 1 << I2C_SR1_OVR);

		//Implement the code to notify the application about the error 
		I2C_ApplicationEventCallback(pI2CHandle, I2C_ERROR_OVR);
	}

/***********************Check for Time out error************************************/
	temp1 = (pI2CHandle->pI2Cx->SR1) & ( 1 << I2C_SR1_TIMEOUT);
	if(temp1  && temp2)
	{
		//This is Time out error
		
	    //Implement the code to clear the Time out error flag
		pI2CHandle->pI2Cx->SR1 &= ~( 1 << I2C_SR1_TIMEOUT);

		//Implement the code to notify the application about the error 
		I2C_ApplicationEventCallback(pI2CHandle, I2C_ERROR_TIMEOUT);
	}

}

uint8_t I2C_MasterSendDataIT_one(I2C_Handle_t *pI2CHandle, uint8_t *pTxBuffer, uint32_t Len, uint8_t SlaveAddr,uint8_t Sr)
{
	uint8_t busystate = pI2CHandle->TxRxState;
	if( (busystate != I2C_BUSY_IN_TX) && (busystate != I2C_BUSY_IN_RX))
	{
		pI2CHandle->pTxBuffer = pTxBuffer;
		pI2CHandle->TxLen = Len;
		pI2CHandle->TxRxState = I2C_BUSY_IN_TX;
		pI2CHandle->DevAddr = SlaveAddr;
		pI2CHandle->Sr = Sr;
		//Implement code to Generate START Condition
	
		I2C_GenerateStartCondition(pI2CHandle->pI2Cx);

		//Implement the code to enable ITBUFEN Control Bit
		pI2CHandle->pI2Cx->CR2 |= ( 1 << I2C_CR2_ITBUFEN);

		//Implement the code to enable ITEVFEN Control Bit
		pI2CHandle->pI2Cx->CR2 |= ( 1 << I2C_CR2_ITEVTEN);

		//Implement the code to enable ITERREN Control Bit
		pI2CHandle->pI2Cx->CR2 |= ( 1 << I2C_CR2_ITERREN);
	}
	return busystate;
}
