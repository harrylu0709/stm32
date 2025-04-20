#ifndef INC_STM32F407XX_TIM_DRIVER_H_
#define INC_STM32F407XX_TIM_DRIVER_H_


#include <stdio.h>
#include "stm32f407xx.h"

typedef struct
{
	TIM_RegDef_t 	*pTIMx;
	uint32_t		Prescalar;
	uint32_t        Reload_Val;
}TIM_Handle_t;

void timer_init(TIM_Handle_t *pTIMHandle);
void TIM_PeriClockControl(TIM_RegDef_t *pTIMx, uint8_t EnorDi);
void TIM_IRQInterruptConfig(uint8_t IRQNumber, uint8_t EnorDi);
void TIM_IRQPriorityConfig(uint8_t IRQNumber, uint32_t IRQPriority);
void TIM_IRQHandling(TIM_Handle_t *pTIM_Handle);
void timer_set_ms(TIM_Handle_t *pTIMHandle, uint16_t period_ms);

#endif
