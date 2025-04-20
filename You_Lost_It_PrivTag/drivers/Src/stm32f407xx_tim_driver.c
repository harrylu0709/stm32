#include "stm32f407xx_tim_driver.h"

void TIM_PeriClockControl(TIM_RegDef_t *pTIMx, uint8_t EnorDi)
{

	if(EnorDi == ENABLE)
	{
		if(pTIMx == TIM2)
		{
			TIM2_PCLK_EN();
		}
        else if (pTIMx == TIM5)
		{
			TIM5_PCLK_EN();
		}
	}
//	else
//	{
//		//TODO
//	}
}
void timer_init(TIM_Handle_t *pTIMHandle)
{
	/* https://embetronicx.com/tutorials/microcontrollers/stm32/simple-stm32-timer-tutorial-bare-metal-with-registers/ */
	/* ensure system clock*/
	//printf("clk=%d\n",RCC_GetPCLK1Value());

	//peripheral clock enable
	TIM_PeriClockControl(pTIMHandle->pTIMx, ENABLE);

  	/* fCK_PSC / (PSC[15:0] + 1)
     (16 MHz / (15999+1)) = 1 KHz timer clock speed */
	pTIMHandle->pTIMx->PSC = pTIMHandle->Prescalar;

  	/* (1 KHz / 1000) = 1Hz = 1s */
  	/* So, this will generate the 1s delay */
	pTIMHandle->pTIMx->ARR = pTIMHandle->Reload_Val;

	// pTIMHandle->pTIMx->DIER |= (1<<6);	
	// pTIMHandle->pTIMx->EGR |= (1<<6);
	//pTIMHandle->pTIMx->EGR |= (1<<0);

	/* Enable the Interrupt */
	pTIMHandle->pTIMx->DIER |= (1<<0);	

	/* Clear the Interrupt Status */
	pTIMHandle->pTIMx->SR &= ~(1<<0);

	/* Enable NVIC Interrupt for Timer */
	TIM_IRQInterruptConfig(IRQ_NO_TIM5, ENABLE);
	TIM_IRQPriorityConfig(IRQ_NO_TIM5, NVIC_IRQ_PRI15);

	/* Finally enable Timer count */
	pTIMHandle->pTIMx->CR1 |= 0x1;
}

void timer_reset(TIM_Handle_t *pTIMHandle)
{
	pTIMHandle->pTIMx->ARR = pTIMHandle->Reload_Val;
}


void TIM_IRQInterruptConfig(uint8_t IRQNumber, uint8_t EnorDi)
{

	if(EnorDi == ENABLE)
	{
		if(IRQNumber <= 31)
		{
			//program ISER0 register
			*NVIC_ISER0 |= ( 1 << IRQNumber );

		}
		else if(IRQNumber > 31 && IRQNumber < 64 ) //32 to 63
		{
			//program ISER1 register
			*NVIC_ISER1 |= ( 1 << (IRQNumber % 32) );
		}
		else if(IRQNumber >= 64 && IRQNumber < 96 )
		{
			//program ISER3 register //64 to 95
			*NVIC_ISER3 |= ( 1 << (IRQNumber % 64) );
		}
	}
	else
	{
		if(IRQNumber <= 31)
		{
			//program ICER0 register
			*NVIC_ICER0 |= ( 1 << IRQNumber );
		}
		else if(IRQNumber > 31 && IRQNumber < 64 )
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

void timer_set_ms(TIM_Handle_t *pTIMHandle, uint16_t period_ms)
{
	pTIMHandle->Reload_Val = period_ms;
}


void TIM_IRQPriorityConfig(uint8_t IRQNumber, uint32_t IRQPriority)
{
	//1. first lets find out the ipr register
	uint8_t iprx = IRQNumber / 4;
	uint8_t iprx_section  = IRQNumber %4 ;

	uint8_t shift_amount = ( 8 * iprx_section) + ( 8 - NO_PR_BITS_IMPLEMENTED) ;

	*(  NVIC_PR_BASE_ADDR + iprx ) |=  ( IRQPriority << shift_amount );
}

void TIM_IRQHandling(TIM_Handle_t *pTIM_Handle)
{
	if(pTIM_Handle->pTIMx->SR & 0x1)
		pTIM_Handle->pTIMx->SR &= ~(1<<0);
}