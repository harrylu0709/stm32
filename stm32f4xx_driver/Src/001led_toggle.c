/*
 * 001led_toggle.c
 *
 *  Created on: 2024年6月23日
 *      Author: bo
 */

#include "stm32f407xx.h"
#define SYSTICK_TIM_CLK   16000000UL

void delay(void)
{
	for(uint32_t i =0; i < 500000 ; i++);

}

void delay_systick(uint32_t tick_hz)
{
	uint32_t *pSRVR = (uint32_t*)0xE000E014;
	uint32_t *pSCSR = (uint32_t*)0xE000E010;
	uint32_t temp;
    /* calculation of reload value */
    uint32_t count_value = (SYSTICK_TIM_CLK/tick_hz)-1;// if 1 second, then tick_hz = 1, if 1 millisecond, then tick_hz = 1000

    //Clear the value of SVR
    *pSRVR &= ~(0x00FFFFFFFF);

	*pSCSR &= ~(0x00FFFFFFFF);

    //load the value in to SVR
    *pSRVR |= count_value;

    //do some settings
    *pSCSR |= ( 1 << 2);  //Indicates the clock source, processor clock source (HSI = 16MHz)

    // *pSCSR |= ( 1 << 1); //Enables SysTick exception request:

    //enable the systick
    *pSCSR |= ( 1 << 0); //enables the counter

	do
	{
		temp = *pSCSR;
	} while ((temp & (0x01) && (!(temp & (1<<16)))));
	/*
	
	SysTick Control and Status Register
	[16] COUNTFLAG : Returns 1 if timer counted to 0 since last time this was read. 
	
	*/
}

int main(void)
{
	GPIO_Handle_t GpioLed;

	GpioLed.pGPIOx = GPIOD;
	GpioLed.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_12;
	GpioLed.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_OUT;
	GpioLed.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;
	GpioLed.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_PP;
	GpioLed.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;

	GPIO_PeriClockControl(GPIOD, ENABLE);

	GPIO_Init(&GpioLed);

	while(1)
	{
		GPIO_ToggleOutputPin(GPIOD, GPIO_PIN_NO_12);
		delay_systick(1);
	}

	return 0;
}
