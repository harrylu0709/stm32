
#include "lcd.h"

GPIO_Handle_t LED;

void led_init(void)
{
	LED.pGPIOx = LED_GPIO_PORT;
	LED.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_OUT;
	LED.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_PP;
	LED.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;
	LED.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;
	LED.GPIO_PinConfig.GPIO_PinNumber = LED_GPIO_RED;
	GPIO_Init(&LED);

	LED.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_OUT;
	LED.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_PP;
	LED.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;
	LED.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;
	LED.GPIO_PinConfig.GPIO_PinNumber = LED_GPIO_GREEN;
	GPIO_Init(&LED);
}

void leds_set(uint8_t led)
{
	if(led & 0x01)
		GPIO_WriteToOutputPin(LED_GPIO_PORT, LED_GPIO_RED, 1);
	else
		GPIO_WriteToOutputPin(LED_GPIO_PORT, LED_GPIO_RED, 0);
	
	if((led>>0) & 0x10)
		GPIO_WriteToOutputPin(LED_GPIO_PORT, LED_GPIO_GREEN, 1);
	else
		GPIO_WriteToOutputPin(LED_GPIO_PORT, LED_GPIO_GREEN, 0);
}