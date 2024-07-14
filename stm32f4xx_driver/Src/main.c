/*
 * main.c
 *
 *  Created on: 2024年6月29日
 *      Author: bo
 */

#include "stm32f407xx.h"


int main(void)
{
	return 0;
}


void EXTI0_IRQHandler(void) //ISR overwite the default ISR in startup_stm32.c
{
	GPIO_IRQHandling(0);
}
