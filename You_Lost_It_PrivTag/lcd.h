#ifndef LED_H_
#define LED_H_

#include "stm32f407xx.h"

void led_init();
void leds_set(uint8_t led);
//Application configurable items
#define LED_GPIO_PORT       GPIOD
#define LED_GPIO_GREEN      GPIO_PIN_NO_12
#define LED_GPIO_ORANGE     GPIO_PIN_NO_13
#define LED_GPIO_RED        GPIO_PIN_NO_14
#define LED_GPIO_BLUE       GPIO_PIN_NO_15

#endif