

#include "stm32f407xx.h"
#include <string.h>

/*
PB14->SPI2_MISO
PB15->SPI2_MOSI
PB13->SPI2_SCLK
PB12->SPI2_NSS
 */

void SPI2_GPIOInits(void)
{
	GPIO_Handle_t SPIPins;

	SPIPins.pGPIOx = GPIOB;
	SPIPins.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_ALTFN;
	SPIPins.GPIO_PinConfig.GPIO_PinAltFunMode = 5;
	SPIPins.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;
	SPIPins.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_PP;
	SPIPins.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;

	//SCLK
	SPIPins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_13;
	GPIO_Init(&SPIPins);

	//MOSI
	SPIPins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_15;
	GPIO_Init(&SPIPins);

//	//MISO
//	SPIPins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_14;
//	GPIO_Init(&SPIPins);
//
//	//NSS
//	SPIPins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_12;
//	GPIO_Init(&SPIPins);
}


void SPI2_Inits(void)
{
	SPI_Handle_t SPI2handle;

	SPI2handle.pSPIx = SPI2;
	SPI2handle.SPI_Config.SPI_BusConfig = SPI_BUS_CONFIG_FD;
	SPI2handle.SPI_Config.SPI_DeviceMode= SPI_DEVICE_MODE_MASTER;
	SPI2handle.SPI_Config.SPI_SclkSpeed= SPI_SCLK_SPEED_DIV2; //generate 8MHz
	SPI2handle.SPI_Config.SPI_DFF = SPI_DFF_8BITS;
	SPI2handle.SPI_Config.SPI_CPOL= SPI_CPOL_LOW;//default
	SPI2handle.SPI_Config.SPI_CPHA= SPI_CPHA_LOW;//default
	SPI2handle.SPI_Config.SPI_SSM= SPI_SSM_EN;//software slave management enabled for NSS pin
	SPI_Init(&SPI2handle);


}

void delay(void)
{
	for(uint32_t i =0; i < 500000 ; i++);

}
int main(void)
{

	char user_data[] = "hello world";

	//this function is used to initialize the GPIO pins to behave as SPI2 pins
	SPI2_GPIOInits();

	//this function is used to initialize SPI2 peripheral parameters
	SPI2_Inits();

	//this make NSS signal internally high and avoids MODF error
	SPI_SSIConfig(SPI2, ENABLE);

	//enable SPI2 Peripheral  //default is disable
	SPI_PeripheralControl(SPI2, ENABLE);

	SPI_SendData(SPI2, (uint8_t*)user_data, strlen(user_data));

	//lets confirm SPI is not busy
	while(SPI_GetFlagStatus(SPI2, SPI_BUSY_FLAG));

	//disable SPI2 Peripheral  //default is disable
	SPI_PeripheralControl(SPI2, DISABLE);

	while(1);
	return 0;
}
