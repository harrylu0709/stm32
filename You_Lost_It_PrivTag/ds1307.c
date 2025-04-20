#include <stdint.h>
#include <string.h>
#include "ds1307.h"


void SPI1_GPIOInits(void)
{
	GPIO_Handle_t SPIPins;

	SPIPins.pGPIOx = GPIOA;
	SPIPins.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_ALTFN;
	SPIPins.GPIO_PinConfig.GPIO_PinAltFunMode = 5;
	SPIPins.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_PP;
	SPIPins.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;
	SPIPins.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_MEDIUM;

	SPIPins.GPIO_PinConfig.GPIO_PinNumber = SPI1_SCK;
	GPIO_Init(&SPIPins);

	SPIPins.GPIO_PinConfig.GPIO_PinNumber = SPI1_MOSI;
	GPIO_Init(&SPIPins);

	SPIPins.GPIO_PinConfig.GPIO_PinNumber = SPI1_MISO;
	GPIO_Init(&SPIPins);

	// SPIPins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_4;
	// GPIO_Init(&SPIPins);

	GPIO_Handle_t CSPins;
	CSPins.pGPIOx = GPIOE;
	CSPins.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_OUT;
	CSPins.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_PP;
	CSPins.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;
	CSPins.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_MEDIUM;
	CSPins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_3;
	GPIO_Init(&CSPins);
}

void SPI1_Inits(void)
{
	SPI_Handle_t SPI1Handle;

	SPI1Handle.pSPIx = SPI1;
	SPI1Handle.SPIConfig.SPI_BusConfig = SPI_BUS_CONFIG_FD;
	SPI1Handle.SPIConfig.SPI_DeviceMode = SPI_DEVICE_MODE_MASTER; 
	SPI1Handle.SPIConfig.SPI_SclkSpeed = SPI_SCLK_SPEED_DIV8;    //generate 2MHz
	SPI1Handle.SPIConfig.SPI_DFF = SPI_DFF_8BITS;
	SPI1Handle.SPIConfig.SPI_CPOL = SPI_CPOL_LOW;
	SPI1Handle.SPIConfig.SPI_CPHA = SPI_CPHA_LOW;
	SPI1Handle.SPIConfig.SPI_SSM = SPI_SSM_EN;
	SPI_Init(&SPI1Handle);
}

void spi_write_reg(uint8_t addr, uint8_t data)
{
	
	uint8_t tx_buf[3];
	tx_buf[0] = WRITE;
	tx_buf[1] = addr;
	tx_buf[2] = data;
	
	GPIO_WriteToOutputPin(GPIOE, SPI1_CS, 0);
	SPI_SendData(SPI1, tx_buf, ARRAY_LENGTH(tx_buf));
	while(SPI_GetFlagStatus(SPI1, SPI_BUSY_FLAG));
	GPIO_WriteToOutputPin(GPIOE, SPI1_CS, 1);
}

void spi_read_reg(uint8_t addr, uint8_t *data)
{
	uint8_t tx_buf[2];
	tx_buf[0] = READ;
	tx_buf[1] = addr;
	SPI_SendData(SPI1, tx_buf, ARRAY_LENGTH(tx_buf));
	SPI_ReceiveData(SPI1, data, 1);
}

void LIS3DSH_init(void)
{
	SPI1_GPIOInits();

	SPI1_Inits();

	//this makes NSS signal internally high and avoids MODF error
	SPI_SSIConfig(SPI1, ENABLE);
	/*
	* making SSOE 1 does NSS output enable.
	* The NSS pin is automatically managed by the hardware.
	* i.e when SPE=1 , NSS will be pulled to low
	* and NSS pin will be high when SPE=0
	*/
	//SPI_SSOEConfig(SPI1,ENABLE);

	SPI_PeripheralControl(SPI1, ENABLE);
	//write CTRL_REG4(0x20) = 0x67; //X,Y,Z enable, ODR = 100Hz
	spi_write_reg(0x20, 0x67);
	//write CTRL_REG3(0x23) = 0xC8; //DRY active high on INT1 pin
	spi_write_reg(0x23, 0xC8);
	
	SPI_PeripheralControl(SPI1, DISABLE);
	//https://stackoverflow.com/questions/50867940/why-am-i-only-getting-0xff-when-reading-from-the-lis3dsh-accelerometer-on-the-st
}

void LIS3DSH_read_xyz(int16_t *x, int16_t *y, int16_t *z)
{
	//status register(0x27)
}
