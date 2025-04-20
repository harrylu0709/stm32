#ifndef DS_1307_H
#define DS_1307_H
#include "stm32f407xx.h"
#define ENABLE_SysTick 0
#if ENABLE_SysTick
	#define ENABLE_Timer 0
#else
	#define ENABLE_Timer 1
#endif
#if ENABLE_Timer
	#define I2C_INT_ENABLE 1
#endif 

#define 	SPI1_SCK 			GPIO_PIN_NO_5
#define 	SPI1_MOSI 			GPIO_PIN_NO_7
#define 	SPI1_MISO 			GPIO_PIN_NO_6
#define 	SPI1_CS 			GPIO_PIN_NO_3 //GPIOE

#define 	MEMS_INT1 			GPIO_PIN_NO_0 //GPIOE
#define 	MEMS_INT2 			GPIO_PIN_NO_1 //GPIOE
#define 	WRITE 				(0<<7)
#define 	READ 				(1<<7)

#define 	ADD_REG_CTRL_4		0x20
#define 	ADD_REG_CTRL_3		0x23
#define 	ADD_REG_OUT_X_L		0x28
#define 	ADD_REG_OUT_X_H		0x29
#define 	ADD_REG_OUT_Y_L		0x2A
#define 	ADD_REG_OUT_Y_H		0x2B
#define 	ADD_REG_OUT_Z_L		0x2C
#define 	ADD_REG_OUT_Z_H		0x2D

#define 	LED_TH_X			(1<<10)
#define 	LED_TH_Y			(1<<10)
#define 	LED_TH_Z			18000

void SPI1_GPIOInits(void);
void SPI1_Inits(void);
void spi_write_reg(uint8_t addr, uint8_t data);
void spi_read_reg(uint8_t addr, uint8_t *data);
void LIS3DSH_init(void);
void LIS3DSH_read_xyz(int16_t *x, int16_t *y, int16_t *z);
#endif