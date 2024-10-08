#include<stdio.h>
#include<string.h>
#include "stm32f407xx.h"

/*
 I2C1
 SCL: PB6, PB8
 SDA: PB7, PB9


 I2C2
 SCL: PB10, PF1
 SDA: PB11, PF0

*/
uint8_t RxComplt = SET;
I2C_Handle_t I2C1Handle;
I2C_Handle_t I2C2Handle;

// uint8_t some_data[] = {0x11,0x22,0x33,0x44};
uint8_t some_data[] = "hello world\n";
#define MY_ADDR 0x61

#define SLAVE_ADDR  0x68

void delay(void)
{
	for(uint32_t i = 0 ; i < 500000/2 ; i ++);
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

void I2C1_GPIOinits(void)
{
	GPIO_Handle_t I2C1Pins;

    I2C1Pins.pGPIOx = GPIOB;
    I2C1Pins.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_ALTFN;
    I2C1Pins.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_OD;
    I2C1Pins.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_PIN_PU;
    I2C1Pins.GPIO_PinConfig.GPIO_PinAltFunMode = 4;
    I2C1Pins.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;

    //scl
    I2C1Pins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_6;
    GPIO_Init(&I2C1Pins);

    // //sda
    // I2CPins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_9;
    // GPIO_Init(&I2CPins);


    // //scl
    // I2CPins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_8;
    // GPIO_Init(&I2CPins);

    //sda
    I2C1Pins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_7;
    GPIO_Init(&I2C1Pins);
}

void I2C2_GPIOinits(void)
{
	GPIO_Handle_t I2C2Pins;

    I2C2Pins.pGPIOx = GPIOB;
    I2C2Pins.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_ALTFN;
    I2C2Pins.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_OD;
    I2C2Pins.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_PIN_PU;
    I2C2Pins.GPIO_PinConfig.GPIO_PinAltFunMode = 4;
    I2C2Pins.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;

    //scl
    I2C2Pins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_10;
    GPIO_Init(&I2C2Pins);

    //sda
    I2C2Pins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_11;
    GPIO_Init(&I2C2Pins);
}

void I2C1_Inits(void)
{
    I2C1Handle.pI2Cx = I2C1;
    I2C1Handle.I2C_Config.I2C_AckControl = I2C_ACK_ENABLE;
    I2C1Handle.I2C_Config.I2C_DeviceAddress = MY_ADDR;
    I2C1Handle.I2C_Config.I2C_FMDutyCycle = I2C_FM_DUTY_2;
    I2C1Handle.I2C_Config.I2C_SCLSpeed = I2C_SCL_SPEED_SM;

    I2C_Init(&I2C1Handle);
}

void I2C2_Inits(void)
{
    I2C2Handle.pI2Cx = I2C2;
    I2C2Handle.I2C_Config.I2C_AckControl = I2C_ACK_ENABLE;
    I2C2Handle.I2C_Config.I2C_DeviceAddress = SLAVE_ADDR;
    I2C2Handle.I2C_Config.I2C_FMDutyCycle = I2C_FM_DUTY_2;
    I2C2Handle.I2C_Config.I2C_SCLSpeed = I2C_SCL_SPEED_SM;

    I2C_Init(&I2C2Handle);
}

int main(void)
{
	// printf("start\n");
    GPIO_ButtonInit();

    I2C1_GPIOinits();

    I2C1_Inits();


    //I2C IRQ configuration
    I2C_IRQInterruptConfig(IRQ_NO_I2C1_EV, ENABLE);
    I2C_IRQInterruptConfig(IRQ_NO_I2C1_ER, ENABLE);
    I2C_PeripheralControl(I2C1, ENABLE);


    //ack bit is made 1 after PE = 1
	I2C_ManageAcking(I2C1, I2C_ACK_ENABLE);


    I2C2_GPIOinits();

    I2C2_Inits();
    //I2C IRQ configuration
    I2C_IRQInterruptConfig(IRQ_NO_I2C2_EV, ENABLE);
    I2C_IRQInterruptConfig(IRQ_NO_I2C2_ER, ENABLE);
    I2C_PeripheralControl(I2C2, ENABLE);

    //ack bit is made 1 after PE = 1
    I2C_ManageAcking(I2C2, I2C_ACK_ENABLE);

    while(1)
    {
 //		//wait till button is pressed
 		while( ! GPIO_ReadFromInputPin(GPIOA,GPIO_PIN_NO_0) );

 		//to avoid button de-bouncing related issues 200ms of delay
 		delay();

 		//send some data to the slave

 		// I2C_MasterSendData(&I2C1Handle,some_data,strlen((char*)some_data), SLAVE_ADDR, 0);
        while(I2C_MasterSendDataIT(&I2C1Handle,some_data,strlen((char*)some_data),SLAVE_ADDR, 0) != I2C_READY);
        // int len = sizeof(some_data)/sizeof(some_data[0]);
        // I2C_MasterSendDataIT(&I2C1Handle,some_data,len,SLAVE_ADDR, 0);

    }
}

void I2C1_EV_IRQHandler(void)
{   
    // printf("i2c1 ev\n");
    I2C_EV_IRQHandling(&I2C1Handle);
}

void I2C1_ER_IRQHandler(void)
{
    // printf("i2c1 ev\n");
    I2C_ER_IRQHandling(&I2C1Handle);
}

void I2C2_EV_IRQHandler(void)
{   
    // printf("i2c2 ev\n");
    I2C_EV_IRQHandling(&I2C2Handle);
}

void I2C2_ER_IRQHandler(void)
{
    // printf("i2c2 er\n");
    I2C_ER_IRQHandling(&I2C2Handle);
}

void I2C_ApplicationEventCallback(I2C_Handle_t *pI2CHandle,uint8_t AppEv)
{
    if(AppEv == I2C_EV_TX_CMPLT)
    {
        printf("Tx is complete\n");
    }
    else if(AppEv == I2C_EV_RX_CMPLT)
    {
        printf("Rx is complete\n");
        RxComplt = SET;
    } 
    else if(AppEv == I2C_ERROR_AF)
    {
        printf("Error: Ack failure\n");

        // in master ack failure happens when slave fails to send ack for the byte
        // sent from master
        I2C_CloseSendData(pI2CHandle);

        // /generate stop condition to release bus
        I2C_GenerateStopCondition(I2C1);

        //Hang in infinite loop
        while(1);
    } 
}
