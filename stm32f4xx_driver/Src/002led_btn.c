///*
// * 001led_toggle.c
// *
// *  Created on: 2024年6月23日
// *      Author: bo
// */
//
//#include "stm32f407xx.h"
//
//void delay(void)
//{
//	for(uint32_t i =0; i < 500000/2 ; i++);
//
//}
//
//int main(void)
//{
//	GPIO_Handle_t GpioLed;
//	GPIO_Handle_t GpioBtn;
//
//
//	GpioLed.pGPIOx = GPIOD;
//	GpioLed.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_12;
//	GpioLed.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_OUT;
//	GpioLed.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;
//	GpioLed.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_PP;
//	GpioLed.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;
//
//	GPIO_PeriClockControl(GPIOD, ENABLE);
//
//	GPIO_Init(&GpioLed);
//
//
//
//	GpioBtn.pGPIOx = GPIOA;
//	GpioBtn.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_0;
//	GpioBtn.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_IN;
//	GpioBtn.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;
//	GpioBtn.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;
//
//	GPIO_PeriClockControl(GPIOA, ENABLE);
//
//	GPIO_Init(&GpioBtn);
//	while(1)
//	{
//		if(GPIO_ReadFromInputPin(GPIOA,GPIO_PIN_NO_0) == 1)
//		{
//			delay();
//			GPIO_ToggleOutputPin(GPIOD, GPIO_PIN_NO_12);
//
//		}
//
//	}
//
//	return 0;
//}
#include "stm32f407xx.h"

#define HIGH 1
#define BTN_PRESSED HIGH

void delay(void)
{
	for(uint32_t i = 0 ; i < 500000/2 ; i ++);
}


int main(void)
{

	GPIO_Handle_t GpioLed, GPIOBtn;

	//this is led gpio configuration
	GpioLed.pGPIOx = GPIOD;
	GpioLed.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_12;
	GpioLed.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_OUT;
	GpioLed.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;
	GpioLed.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_PP;
	GpioLed.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;

	GPIO_PeriClockControl(GPIOD,ENABLE);

	GPIO_Init(&GpioLed);


	//this is btn gpio configuration
	GPIOBtn.pGPIOx = GPIOA;
	GPIOBtn.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_0;
	GPIOBtn.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_IN;
	GPIOBtn.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;
	GPIOBtn.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;

	GPIO_PeriClockControl(GPIOA,ENABLE);

	GPIO_Init(&GPIOBtn);

	while(1)
	{
		if(GPIO_ReadFromInputPin(GPIOA,GPIO_PIN_NO_0) == BTN_PRESSED)
		{
			delay();
			GPIO_ToggleOutputPin(GPIOD,GPIO_PIN_NO_12);
//			delay();
		}
	}
	return 0;
}
