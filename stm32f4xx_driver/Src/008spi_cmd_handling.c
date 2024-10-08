#include <stdio.h>
#include <string.h>
#include "stm32f407xx.h"
//extern void initialise_monitor_handles();
/*
PB14->SPI2_MISO
PB15->SPI2_MOSI
PB13->SPI2_SCLK
PB12->SPI2_NSS


PA6->SPI1_MISO
PA7->SPI1_MOSI
PA5->SPI1_SCLK
PA4->SPI1_NSS

 */

#define COMMAND_LED_CTRL  		0x50
#define COMMAND_SENSOR_READ  	0x51
#define COMMAND_LED_READ  		0x52
#define COMMAND_PRINT  			0x53
#define COMMAND_ID_READ  		0x54


#define LED_ON     1
#define LED_OFF    0

//arduino analog pins
#define ANALOG_PIN0 	0
#define ANALOG_PIN1 	1
#define ANALOG_PIN2 	2
#define ANALOG_PIN3 	3
#define ANALOG_PIN4 	4

//arduino led

#define LED_PIN  9

void delay(void)
{
	for(uint32_t i = 0 ; i < 500000/2 ; i ++);
}
void SPI1_GPIOInits(void)
{
	GPIO_Handle_t SPIPins;

	SPIPins.pGPIOx = GPIOA;
	SPIPins.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_ALTFN;
	SPIPins.GPIO_PinConfig.GPIO_PinAltFunMode = 5;
	SPIPins.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;
	SPIPins.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_PP;
	SPIPins.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;

	//SCLK
	SPIPins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_5;
	GPIO_Init(&SPIPins);

	//MOSI
	SPIPins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_7;
	GPIO_Init(&SPIPins);

	//MISO
	SPIPins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_6;
	GPIO_Init(&SPIPins);

	//NSS
	SPIPins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_4;
	GPIO_Init(&SPIPins);
}

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

	//MISO
	SPIPins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_14;
	GPIO_Init(&SPIPins);

	//NSS
	SPIPins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_12;
	GPIO_Init(&SPIPins);
}

void GPIO_ButtonInit(void)
{
	GPIO_Handle_t GPIOBtn;//,GpioLed;

	//this is btn gpio configuration
	GPIOBtn.pGPIOx = GPIOA;
	GPIOBtn.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_0;
	GPIOBtn.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_IN;
	GPIOBtn.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;
	GPIOBtn.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;

	GPIO_Init(&GPIOBtn);

//	//this is led gpio configuration
//	GpioLed.pGPIOx = GPIOD;
//	GpioLed.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_12;
//	GpioLed.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_OUT;
//	GpioLed.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;
//	GpioLed.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_OD;
//	GpioLed.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;
//
//	GPIO_PeriClockControl(GPIOD,ENABLE);
//
//	GPIO_Init(&GpioLed);

}

void SPI1_Inits(void)
{
	SPI_Handle_t SPI1handle;

	SPI1handle.pSPIx = SPI1;
	SPI1handle.SPIConfig.SPI_BusConfig = SPI_BUS_CONFIG_FD;
	SPI1handle.SPIConfig.SPI_DeviceMode= SPI_DEVICE_MODE_SLAVE;
	SPI1handle.SPIConfig.SPI_SclkSpeed= SPI_SCLK_SPEED_DIV4; //generate 8MHz
	SPI1handle.SPIConfig.SPI_DFF = SPI_DFF_8BITS;
	SPI1handle.SPIConfig.SPI_CPOL= SPI_CPOL_LOW;//default
	SPI1handle.SPIConfig.SPI_CPHA= SPI_CPHA_LOW;//default
	SPI1handle.SPIConfig.SPI_SSM= SPI_SSM_EN;//software slave management enabled for NSS pin
	SPI_Init(&SPI1handle);
}

void SPI2_Inits(void)
{
	SPI_Handle_t SPI2handle;

	SPI2handle.pSPIx = SPI2;
	SPI2handle.SPIConfig.SPI_BusConfig = SPI_BUS_CONFIG_FD;
	SPI2handle.SPIConfig.SPI_DeviceMode= SPI_DEVICE_MODE_MASTER;
	SPI2handle.SPIConfig.SPI_SclkSpeed= SPI_SCLK_SPEED_DIV4; //generate 8MHz
	SPI2handle.SPIConfig.SPI_DFF = SPI_DFF_8BITS;
	SPI2handle.SPIConfig.SPI_CPOL= SPI_CPOL_LOW;//default
	SPI2handle.SPIConfig.SPI_CPHA= SPI_CPHA_LOW;//default
	SPI2handle.SPIConfig.SPI_SSM= SPI_SSM_EN;//software slave management enabled for NSS pin
	SPI_Init(&SPI2handle);
}

uint8_t SPI_VerifyResponse(uint8_t ackbyte)
{
	if(ackbyte == (uint8_t)0xF5)
	{
		return 1;
	}
	return 0;
}
int main(void)
{
	uint8_t dummy_write = 0xff;
	uint8_t dummy_read;

//	initialise_monitor_handles();

	printf("Application is running\n");

	GPIO_ButtonInit();

	//this function is used to initialize the GPIO pins to behave as SPI2 pins
	SPI2_GPIOInits();

	SPI1_GPIOInits();
	//This function is used to initialize the SPI2 peripheral parameters
	SPI2_Inits();

	SPI1_Inits();
//	printf("SPI Init. done\n");

	/*
	* making SSOE 1 does NSS output enable.
	* The NSS pin is automatically managed by the hardware.
	* i.e when SPE=1 , NSS will be pulled to low
	* and NSS pin will be high when SPE=0
	*/
	SPI_SSOEConfig(SPI2,ENABLE);
	SPI_SSOEConfig(SPI1,1);
	while(1)
	{
		//wait till button is pressed
		while( ! GPIO_ReadFromInputPin(GPIOA,GPIO_PIN_NO_0) );

		//to avoid button de-bouncing related issues 200ms of delay
		delay();

		//enable the SPI2 peripheral
		SPI_PeripheralControl(SPI2,ENABLE);

		SPI_PeripheralControl(SPI1,ENABLE);
	    //1. CMD_LED_CTRL  	<pin no(1)>     <value(1)>

		uint8_t commandcode = 0x50;
		uint8_t slave_rx = 0x00;


		//send command
		SPI_SendData(SPI2,&commandcode,1);

		//do dummy read to clear off the RXNE
//		SPI_ReceiveData(SPI2,&dummy_read,1);

//		printf("command code %x\n",commandcode);
//		SPI_SendData(SPI1,&dummy_write,1);
		SPI_ReceiveData(SPI1,&slave_rx,1);

//		printf("slave rx %x\n",slave_rx);
//		delay();
//		SPI_SendData(SPI1,&slave_tx,1);
//
////		delay();
//		//Send some dummy bits (1 byte) fetch the response from the slave
////		SPI_SendData(SPI2,&dummy_write,1);
//
//		//read the ack byte received
//		SPI_ReceiveData(SPI2,&ackbyte,1);
//
//		if( SPI_VerifyResponse(ackbyte))
//		{
//			args[0] = LED_PIN;
//			args[1] = LED_ON;
//
//			//send arguments
//			SPI_SendData(SPI2,args,2);
//			// dummy read
//			SPI_ReceiveData(SPI2,args,2);
////			printf("COMMAND_LED_CTRL Executed\n");
//		}
		//end of COMMAND_LED_CTRL

		//lets confirm SPI is not busy
		while( SPI_GetFlagStatus(SPI2,SPI_BUSY_FLAG) );
		while( SPI_GetFlagStatus(SPI1,SPI_BUSY_FLAG) );
		//Disable the SPI2 peripheral
		SPI_PeripheralControl(SPI2,DISABLE);
		SPI_PeripheralControl(SPI1,DISABLE);
//		printf("SPI Communication Closed\n");
	}

	return 0;

}
//
//#include <stdio.h>
//#include <string.h>
//#include "stm32f407xx.h"
////extern void initialise_monitor_handles();
///*
//PB14->SPI2_MISO
//PB15->SPI2_MOSI
//PB13->SPI2_SCLK
//PB12->SPI2_NSS
//
//
//PA6->SPI1_MISO
//PA7->SPI1_MOSI
//PA5->SPI1_SCLK
//PA4->SPI1_NSS
//
// */
//
//#define COMMAND_LED_CTRL  		0x50
//#define COMMAND_SENSOR_READ  	0x51
//#define COMMAND_LED_READ  		0x52
//#define COMMAND_PRINT  			0x53
//#define COMMAND_ID_READ  		0x54
//
//
//#define LED_ON     1
//#define LED_OFF    0
//
////arduino analog pins
//#define ANALOG_PIN0 	0
//#define ANALOG_PIN1 	1
//#define ANALOG_PIN2 	2
//#define ANALOG_PIN3 	3
//#define ANALOG_PIN4 	4
//
////arduino led
//
//#define LED_PIN  9
//
//void delay(void)
//{
//	for(uint32_t i = 0 ; i < 500000/2 ; i ++);
//}
//void SPI1_GPIOInits(void)
//{
//	GPIO_Handle_t SPIPins;
//
//	SPIPins.pGPIOx = GPIOA;
//	SPIPins.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_ALTFN;
//	SPIPins.GPIO_PinConfig.GPIO_PinAltFunMode = 5;
//	SPIPins.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;
//	SPIPins.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_PP;
//	SPIPins.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;
//
//	//SCLK
//	SPIPins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_5;
//	GPIO_Init(&SPIPins);
//
//	//MOSI
//	SPIPins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_7;
//	GPIO_Init(&SPIPins);
//
//	//MISO
//	SPIPins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_6;
//	GPIO_Init(&SPIPins);
//
//	//NSS
//	SPIPins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_4;
//	GPIO_Init(&SPIPins);
//}
//
//void SPI2_GPIOInits(void)
//{
//	GPIO_Handle_t SPIPins;
//
//	SPIPins.pGPIOx = GPIOB;
//	SPIPins.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_ALTFN;
//	SPIPins.GPIO_PinConfig.GPIO_PinAltFunMode = 5;
//	SPIPins.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;
//	SPIPins.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_PP;
//	SPIPins.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;
//
//	//SCLK
//	SPIPins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_13;
//	GPIO_Init(&SPIPins);
//
//	//MOSI
//	SPIPins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_15;
//	GPIO_Init(&SPIPins);
//
//	//MISO
//	SPIPins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_14;
//	GPIO_Init(&SPIPins);
//
//	//NSS
//	SPIPins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_12;
//	GPIO_Init(&SPIPins);
//}
//
//void GPIO_ButtonInit(void)
//{
//	GPIO_Handle_t GPIOBtn,GpioLed;
//
//	//this is btn gpio configuration
//	GPIOBtn.pGPIOx = GPIOA;
//	GPIOBtn.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_0;
//	GPIOBtn.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_IN;
//	GPIOBtn.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;
//	GPIOBtn.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;
//
//	GPIO_Init(&GPIOBtn);
//
////	//this is led gpio configuration
////	GpioLed.pGPIOx = GPIOD;
////	GpioLed.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_12;
////	GpioLed.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_OUT;
////	GpioLed.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;
////	GpioLed.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_OD;
////	GpioLed.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;
////
////	GPIO_PeriClockControl(GPIOD,ENABLE);
////
////	GPIO_Init(&GpioLed);
//
//}
//
//void SPI1_Inits(int mode)
//{
//	SPI_Handle_t SPI1handle;
//
//	SPI1handle.pSPIx = SPI1;
//	SPI1handle.SPIConfig.SPI_BusConfig = SPI_BUS_CONFIG_FD;
//	SPI1handle.SPIConfig.SPI_DeviceMode= mode;//SPI_DEVICE_MODE_SLAVE;
//	SPI1handle.SPIConfig.SPI_SclkSpeed= SPI_SCLK_SPEED_DIV4; //generate 8MHz
//	SPI1handle.SPIConfig.SPI_DFF = SPI_DFF_8BITS;
//	SPI1handle.SPIConfig.SPI_CPOL= SPI_CPOL_LOW;//default
//	SPI1handle.SPIConfig.SPI_CPHA= SPI_CPHA_LOW;//default
//	SPI1handle.SPIConfig.SPI_SSM= SPI_SSM_EN;//software slave management enabled for NSS pin
//	SPI_Init(&SPI1handle);
//}
//
//void SPI2_Inits(int mode)
//{
//	SPI_Handle_t SPI2handle;
//
//	SPI2handle.pSPIx = SPI2;
//	SPI2handle.SPIConfig.SPI_BusConfig = SPI_BUS_CONFIG_FD;
//	SPI2handle.SPIConfig.SPI_DeviceMode= mode;//SPI_DEVICE_MODE_MASTER;
//	SPI2handle.SPIConfig.SPI_SclkSpeed= SPI_SCLK_SPEED_DIV4; //generate 8MHz
//	SPI2handle.SPIConfig.SPI_DFF = SPI_DFF_8BITS;
//	SPI2handle.SPIConfig.SPI_CPOL= SPI_CPOL_LOW;//default
//	SPI2handle.SPIConfig.SPI_CPHA= SPI_CPHA_LOW;//default
//	SPI2handle.SPIConfig.SPI_SSM= SPI_SSM_EN;//software slave management enabled for NSS pin
//	SPI_Init(&SPI2handle);
//}
//
//uint8_t SPI_VerifyResponse(uint8_t ackbyte)
//{
//	if(ackbyte == (uint8_t)0xF5)
//	{
//		return 1;
//	}
//	return 0;
//}
//int main(void)
//{
//	uint8_t dummy_write = 0xff;
//	uint8_t dummy_read;
//
////	initialise_monitor_handles();
//
//	printf("Application is running\n");
//
//	GPIO_ButtonInit();
//
//	//this function is used to initialize the GPIO pins to behave as SPI2 pins
//	SPI1_GPIOInits();
//
//	SPI2_GPIOInits();
//	//This function is used to initialize the SPI2 peripheral parameters
//	SPI2_Inits(1);
//
//	SPI1_Inits(0);
////	printf("SPI Init. done\n");
//
//	/*
//	* making SSOE 1 does NSS output enable.
//	* The NSS pin is automatically managed by the hardware.
//	* i.e when SPE=1 , NSS will be pulled to low
//	* and NSS pin will be high when SPE=0
//	*/
//	SPI_SSOEConfig(SPI2,ENABLE);
//	SPI_SSOEConfig(SPI1,1);
//	while(1)
//	{
//		//wait till button is pressed
//		while( ! GPIO_ReadFromInputPin(GPIOA,GPIO_PIN_NO_0) );
//
//		//to avoid button de-bouncing related issues 200ms of delay
//		delay();
//
//		//enable the SPI2 peripheral
//		SPI_PeripheralControl(SPI2,ENABLE);
//
//		SPI_PeripheralControl(SPI1,ENABLE);
//	    //1. CMD_LED_CTRL  	<pin no(1)>     <value(1)>
//
//		uint8_t commandcode = 0x50;
//		uint8_t slave_rx = 0x00;
//		uint8_t slave_tx = 0x00;
//		uint8_t ack_data = 0x00;
//
////		SPI_SendData(SPI1,&slave_tx,1);
////		printf("slave tx %x\n",slave_tx);
//////		SPI_ReceiveData(SPI1,&dummy_read,1);
//////		SPI_ReceiveData(SPI1,&dummy_read,1);
////		printf("dummy\n");
//////
////////		delay();
//////		//Send some dummy bits (1 byte) fetch the response from the slave
//////
//////		//read the ack byte received
////		SPI_ReceiveData(SPI2,&ack_data,1);
////		printf("ackbyte %x\n",ack_data);
////		return 0;
//
////		SPI1->DR = 0x51;
//		SPI_SendData(SPI2,&commandcode,1);
//
//		//do dummy read to clear off the RXNE
////		SPI_ReceiveData(SPI2,&dummy_read,1);
//
//		printf("command code %x\n",commandcode);
////		SPI_SendData(SPI1,&dummy_write,1);
//		SPI_ReceiveData(SPI1,&slave_rx,1);
//		printf("slave rx %x\n",slave_rx);
//
////		return 0;
////		SPI2_Inits(0);
////
////		SPI1_Inits(1);
////		SPI2_GPIOInits();
////
////		SPI1_GPIOInits();
//
//
//
////		delay();
////		SPI_ReceiveData(SPI1,&dummy_read,1);
////		printf("dummy \n");
////		SPI_SendData(SPI1,&dummy_write,1);
////		printf("dummy\n");
//
////		SPI_SendData(SPI1,&slave_tx,1);
////		printf("slave tx %x\n",slave_tx);
//////		SPI_ReceiveData(SPI1,&dummy_read,1);
//////		SPI_SendData(SPI2,&dummy_write,1);
////		printf("dummy\n");
//////
////////		delay();
//////		//Send some dummy bits (1 byte) fetch the response from the slave
//////		SPI_SendData(SPI2,&dummy_write,1);
//////
//////		//read the ack byte received
////		SPI_ReceiveData(SPI2,&ack_data,1);
////		printf("ackbyte %x\n",ack_data);
//
//
//
////
////		if( SPI_VerifyResponse(ackbyte))
////		{
////			args[0] = LED_PIN;
////			args[1] = LED_ON;
////
////			//send arguments
////			SPI_SendData(SPI2,args,2);
////			// dummy read
////			SPI_ReceiveData(SPI2,args,2);
//////			printf("COMMAND_LED_CTRL Executed\n");
////		}
//		//end of COMMAND_LED_CTRL
//
//		//lets confirm SPI is not busy
//		while( SPI_GetFlagStatus(SPI2,SPI_BUSY_FLAG) );
//		while( SPI_GetFlagStatus(SPI1,SPI_BUSY_FLAG) );
//		//Disable the SPI2 peripheral
//		SPI_PeripheralControl(SPI2,DISABLE);
//		SPI_PeripheralControl(SPI1,DISABLE);
////		printf("SPI Communication Closed\n");
//	}
//
//
//
//	return 0;
//
//}
////int main(void)
////{
////	uint8_t dummy_write = 0xff;
////	uint8_t dummy_read;
////
////	GPIO_ButtonInit();
////
////	//this function is used to initialize the GPIO pins to behave as SPI2 pins
////	SPI2_GPIOInits();
//////	SPI1_GPIOInits();
////	//this function is used to initialize SPI2 peripheral parameters
////	SPI2_Inits();
//////	SPI1_Inits();
////	/*
////	 making SSOE 1 does NSS output enable
////	 NSS pin is automatically managed by the hardware
////	 i.e. when SPE = 1, NSS will be pulled to low
////	 and NSS pin will be high when SPE = 0
////	 */
////	SPI_SSOEConfig(SPI2, ENABLE);
////
////	while(1)
////	{
////		//wait until button is pressed
////		while(!GPIO_ReadFromInputPin(GPIOA, GPIO_PIN_NO_0));
////
////		//to avoid button de-bouncing
////		delay();
////
////		//enable SPI2 Peripheral  //default is disable
////		SPI_PeripheralControl(SPI2, ENABLE);
////
////		uint8_t commandcode = COMMAND_LED_CTRL;
////		uint8_t ackbyte;
////		uint8_t args[2];
////
////		//send command
////		SPI_SendData(SPI2,&commandcode,1);
////		/*
////		* Remember
////		In SPI communication when Master or Slave sends 1 byte,
////		it also receives 1 byte in return
////
////		* The transmission of 1 byte resulted 1 garbage byte collection in Rx buffer
////		of the master and RXNE flag is set. So do the dummy read and clear the flag
////		 */
////		//do dummy read to clear off the RXNE
////		SPI_ReceiveData(SPI2,&dummy_read,1);
////
////
////		/*
////		 Send some dummy bits (1 byte) fetch the response from the slave
////		 Slave ack is already in shift register
////		 in order to move that datat out of  the slave shift register,
////		 you have  to send some dummy byte
////		 */
////
////		SPI_SendData(SPI2,&dummy_write,1);
////
////		//read the ack byte received
////		SPI_ReceiveData(SPI2,&ackbyte,1);
////
////		if( SPI_VerifyResponse(ackbyte))
////		{
////			args[0] = LED_PIN;
////			args[1] = LED_ON;
////
////			//send arguments
////			SPI_SendData(SPI2,args,2);
////			// dummy read
////			SPI_ReceiveData(SPI2,args,2);
//////			printf("COMMAND_LED_CTRL Executed\n");
////		}
////		//end of COMMAND_LED_CTRL
////	}
////
////
////
//////	SPI_SendData(SPI2, (uint8_t*)user_data, strlen(user_data));
////
////	//lets confirm SPI is not busy
////	while(SPI_GetFlagStatus(SPI2, SPI_BUSY_FLAG));
////
////	//disable SPI2 Peripheral  //default is disable
////	SPI_PeripheralControl(SPI2, DISABLE);
////
////	return 0;
////}
//
////
/////*
//// * 006spi_tx_testing.c
//// *
//// *  Created on: Feb 10, 2019
//// *      Author: admin
//// */
////
////#include<string.h>
////#include "stm32f407xx.h"
////
////void delay(void)
////{
////	for(uint32_t i = 0 ; i < 500000 ; i ++);
////}
////
/////*
//// * PB14 --> SPI2_MISO
//// * PB15 --> SPI2_MOSI
//// * PB13 -> SPI2_SCLK
//// * PB12 --> SPI2_NSS
//// * ALT function mode : 5
//// */
////
/////* SPI Slave Demo
////
//// *
//// * SPI pin numbers:
//// * SCK   13  // Serial Clock.
//// * MISO  12  // Master In Slave Out.
//// * MOSI  11  // Master Out Slave In.
//// * SS    10  // Slave Select . Arduino SPI pins respond only if SS pulled low by the master
//// *
////
//// */
////
////void SPI2_GPIOInits(void)
////{
////	GPIO_Handle_t SPIPins;
////
////	SPIPins.pGPIOx = GPIOB;
////	SPIPins.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_ALTFN;
////	SPIPins.GPIO_PinConfig.GPIO_PinAltFunMode = 5;
////	SPIPins.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_PP;
////	SPIPins.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;
////	SPIPins.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;
////
////	//SCLK
////	SPIPins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_13;
////	GPIO_Init(&SPIPins);
////
////	//MOSI
////	SPIPins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_15;
////	GPIO_Init(&SPIPins);
////
////	//MISO
////	//SPIPins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_14;
////	//GPIO_Init(&SPIPins);
////
////
////	//NSS
////	SPIPins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_12;
////	GPIO_Init(&SPIPins);
////
////
////}
////
////void SPI2_Inits(void)
////{
////
////	SPI_Handle_t SPI2handle;
////
////	SPI2handle.pSPIx = SPI2;
////	SPI2handle.SPIConfig.SPI_BusConfig = SPI_BUS_CONFIG_FD;
////	SPI2handle.SPIConfig.SPI_DeviceMode = SPI_DEVICE_MODE_MASTER;
////	SPI2handle.SPIConfig.SPI_SclkSpeed = SPI_SCLK_SPEED_DIV32;
////	SPI2handle.SPIConfig.SPI_DFF = SPI_DFF_8BITS;
////	SPI2handle.SPIConfig.SPI_CPOL = SPI_CPOL_LOW;
////	SPI2handle.SPIConfig.SPI_CPHA = SPI_CPHA_LOW;
////	SPI2handle.SPIConfig.SPI_SSM = SPI_SSM_DI; //Hardware slave management enabled for NSS pin
////
////	SPI_Init(&SPI2handle);
////}
////
////void GPIO_ButtonInit(void)
////{
////	GPIO_Handle_t GPIOBtn;
////
////	//this is btn gpio configuration
////	GPIOBtn.pGPIOx = GPIOA;
////	GPIOBtn.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_0;
////	GPIOBtn.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_IN;
////	GPIOBtn.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;
////	GPIOBtn.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;
////
////	GPIO_Init(&GPIOBtn);
////
////}
////
////
////int main(void)
////{
//////	char user_data[] = "An Arduino Uno board is best suited for beginners who have just started using microcontrollers, on the other hand, Arduino Mega board is for enthusiasts who require a lot of I/O pins for their projects";
////	char user_data[] = "Hello World";
////	GPIO_ButtonInit();
////
////	//this function is used to initialize the GPIO pins to behave as SPI2 pins
////	SPI2_GPIOInits();
////
////	//This function is used to initialize the SPI2 peripheral parameters
////	SPI2_Inits();
////
////	/*
////	* making SSOE 1 does NSS output enable.
////	* The NSS pin is automatically managed by the hardware.
////	* i.e when SPE=1 , NSS will be pulled to low
////	* and NSS pin will be high when SPE=0
////	*/
////	SPI_SSOEConfig(SPI2,ENABLE);
////
////	while(1)
////	{
////		//wait till button is pressed
////		while( ! GPIO_ReadFromInputPin(GPIOA,GPIO_PIN_NO_0) );
////
////		//to avoid button de-bouncing related issues 200ms of delay
////		delay();
////
////		//enable the SPI2 peripheral
////		SPI_PeripheralControl(SPI2,ENABLE);
////
////		//first send length information
////		uint8_t dataLen = strlen(user_data);
////		SPI_SendData(SPI2,&dataLen,1);
////
////		//to send data
////		SPI_SendData(SPI2,(uint8_t*)user_data,strlen(user_data));
////
////		//lets confirm SPI is not busy
////		while( SPI_GetFlagStatus(SPI2,SPI_BUSY_FLAG) );
////
////		//Disable the SPI2 peripheral
////		SPI_PeripheralControl(SPI2,DISABLE);
////	}
////
////	return 0;
////
////}
