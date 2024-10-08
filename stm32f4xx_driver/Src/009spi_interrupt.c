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
SPI_Handle_t SPI1handle;
SPI_Handle_t SPI2handle;

#define MAXLEN 500

char RcvBuff[MAXLEN];

volatile uint8_t commandcode = 0x50;
volatile uint8_t slave_rx = 0x00;

volatile uint8_t rcvStop = 0;
volatile uint8_t dataAvailable = 0;

uint8_t btn_flag = 0;
//arduino led


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

}

void SPI1_Inits(void)
{
//	SPI_Handle_t SPI1handle;

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

void SPI2_GPIO_InterruptPinInit(void)
{
	GPIO_Handle_t spiIntPin;
	memset(&spiIntPin,0,sizeof(spiIntPin));
	spiIntPin.pGPIOx = GPIOD;
	spiIntPin.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_6;
	spiIntPin.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_IT_FT;
	spiIntPin.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_LOW;
	spiIntPin.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;

	GPIO_PeriClockControl(GPIOD,ENABLE);

	GPIO_Init(&spiIntPin);
	//IRQ configuartions
	GPIO_IRQPriorityConfig(IRQ_NO_EXTI9_5, NVIC_IRQ_PRI15);
	GPIO_IRQInterruptConfig(IRQ_NO_EXTI9_5, ENABLE);
}

void GPIO_Btn_InterruptPinInit(void)
{
	GPIO_Handle_t GPIOBtn;
	memset(&GPIOBtn, 0 , sizeof(GPIOBtn));
	GPIOBtn.pGPIOx = GPIOA;
	GPIOBtn.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_0;
	GPIOBtn.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_IT_FT;
	GPIOBtn.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;
	GPIOBtn.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;

	GPIO_PeriClockControl(GPIOA,ENABLE);

	GPIO_Init(&GPIOBtn);
	//IRQ configuartions
	GPIO_IRQPriorityConfig(IRQ_NO_EXTI0, NVIC_IRQ_PRI15);
	GPIO_IRQInterruptConfig(IRQ_NO_EXTI0, ENABLE);
}
int main(void)
{
	uint8_t dummy_write = 0xFF;
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
	SPI_SSOEConfig(SPI1,ENABLE);

	SPI2_GPIO_InterruptPinInit();
	GPIO_Btn_InterruptPinInit();

	SPI_IRQInterruptConfig(IRQ_NO_SPI2,ENABLE);
	SPI_IRQInterruptConfig(IRQ_NO_SPI1,ENABLE);

	while(1)
	{
		while(!btn_flag);
		GPIO_IRQInterruptConfig(IRQ_NO_EXTI0,DISABLE);
		GPIO_IRQInterruptConfig(IRQ_NO_EXTI9_5,DISABLE);
		SPI_PeripheralControl(SPI2,ENABLE);
		SPI_PeripheralControl(SPI1,ENABLE);

		while(!dataAvailable);

		printf("send\n");
		while(SPI_SendDataIT(&SPI2handle,&commandcode, 1) == SPI_BUSY_IN_TX);
//		SPI_SendData(SPI1,&dummy_write,1);
//		SPI_ReceiveData(SPI1,&slave_rx,1);
		printf("rx %x\n",slave_rx);
		while(SPI_SendDataIT(&SPI1handle,&dummy_write, 1) == SPI_BUSY_IN_TX);
		while(SPI_ReceiveDataIT(&SPI1handle,&slave_rx, 1) == SPI_BUSY_IN_RX);
////
//////		printf("dr %x\n",SPI2handle.pSPIx->DR);
//		printf("rx %x\n",slave_rx);
//		printf("rxne %d\n",((SPI1handle.pSPIx->CR2)>>6)&1);

//		if(btn_flag && dataAvailable)
//		{
//			printf("a %d\n",dataAvailable);
//			GPIO_IRQInterruptConfig(IRQ_NO_EXTI0,DISABLE);
//			SPI_PeripheralControl(SPI2,ENABLE);
//			SPI_PeripheralControl(SPI1,ENABLE);
////			SPI_SendData(SPI2,&commandcode,1);
////			SPI_SendDataIT(&SPI2handle,&commandcode, 1);
//			while(SPI_SendDataIT(&SPI2handle,&commandcode, 1) == SPI_BUSY_IN_TX);
////			SPI_SendData(SPI1,&dummy_write,1);
////			SPI_ReceiveData(SPI1,&slave_rx,1);
////			printf("slave_rx %x\n",slave_rx);
//			GPIO_IRQInterruptConfig(IRQ_NO_EXTI0,ENABLE);
//		}
		btn_flag = 0;
		dataAvailable = 0;

		while( SPI_GetFlagStatus(SPI2,SPI_BUSY_FLAG) );
		while( SPI_GetFlagStatus(SPI1,SPI_BUSY_FLAG) );
		//Disable the SPI2 peripheral
		SPI_PeripheralControl(SPI2,DISABLE);
		SPI_PeripheralControl(SPI1,DISABLE);

		GPIO_IRQInterruptConfig(IRQ_NO_EXTI0,ENABLE);
		GPIO_IRQInterruptConfig(IRQ_NO_EXTI9_5,ENABLE);
	}
	return 0;
}


void SPI2_IRQHandler(void)
{
	printf("spi2 irq\n");
	SPI_IRQHandling(&SPI2handle);
}


void SPI1_IRQHandler(void)
{
	printf("spi1 irq\n");
	SPI_IRQHandling(&SPI1handle);
}


void EXTI0_IRQHandler(void)//ISR
{

	delay();
	GPIO_IRQHandling(GPIO_PIN_NO_0); //clear the pending event from EXTI line
	btn_flag = 1;
//	SPI_PeripheralControl(SPI2,ENABLE);
//	SPI_SendData(SPI2,&commandcode,1);
//	SPI_SendData(SPI1,&dummy_write,1);
//	SPI_ReceiveData(SPI1,&slave_rx,1);
//	while(SPI_SendDataIT(&SPI2handle,&commandcode, 1) == SPI_BUSY_IN_TX);
//	GPIO_ToggleOutputPin(GPIOD, GPIO_PIN_NO_12);
}

void EXTI9_5_IRQHandler(void)//ISR
{
	GPIO_IRQHandling(GPIO_PIN_NO_6); //clear the pending event from EXTI line
	dataAvailable = 1;
}
