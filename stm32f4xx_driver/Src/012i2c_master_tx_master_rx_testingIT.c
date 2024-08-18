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

// uint8_t some_data[] = "hello world\n";
uint8_t some_data[] = "slave send\n";

uint8_t rcv_buf[32];
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
    uint8_t commandcode;
    uint8_t len;
	// printf("start\n");
    GPIO_ButtonInit();

    I2C1_GPIOinits();

    I2C1_Inits();


    //I2C IRQ configuration
    I2C_IRQInterruptConfig(IRQ_NO_I2C1_EV, ENABLE);
    I2C_IRQInterruptConfig(IRQ_NO_I2C1_ER, ENABLE);
    I2C_PeripheralControl(I2C1, ENABLE);
    // I2C_IRQPriorityConfig();

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
        // while(I2C_MasterSendDataIT(&I2C1Handle,some_data,strlen((char*)some_data),SLAVE_ADDR, 0) != I2C_READY);
        // int len = sizeof(some_data)/sizeof(some_data[0]);
        // I2C_MasterSendDataIT(&I2C1Handle,some_data,len,SLAVE_ADDR, 0);

        commandcode = 0x51;
        // printf("1\n");
        while(I2C_MasterSendDataIT(&I2C1Handle, &commandcode, 1, SLAVE_ADDR, I2C_ENABLE_SR) != I2C_READY);
        // printf("2\n");
		while(I2C_MasterReceiveDataIT(&I2C1Handle, &len, 1, SLAVE_ADDR, I2C_ENABLE_SR)!= I2C_READY);
        // printf("3\n");

		commandcode = 0x52;
		while(I2C_MasterSendDataIT(&I2C1Handle, &commandcode, 1, SLAVE_ADDR, I2C_ENABLE_SR) != I2C_READY);
		while(I2C_MasterReceiveDataIT(&I2C1Handle, rcv_buf,len, SLAVE_ADDR, I2C_DISABLE_SR)!= I2C_READY);

		RxComplt = RESET;

		// //wait till rx completes
        while(RxComplt != SET)
        {
            
        }

		rcv_buf[len+1] = '\0';

		printf("Data : %s",rcv_buf);

		RxComplt = RESET;
    }
}

void I2C1_EV_IRQHandler(void)
{   
    // printf("i2c1 ev\n");
    I2C_EV_IRQHandling(&I2C1Handle);
}

void I2C1_ER_IRQHandler(void)
{
    // printf("i2c1 er\n");
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

#if 1
void I2C_ApplicationEventCallback(I2C_Handle_t *pI2CHandle,uint8_t AppEv)
{
    static uint8_t commandCode = 0;
    static uint8_t cnt = 0;
    if(AppEv == I2C_EV_DATA_REQ)
    {
        // // printf("Tx is complete\n");
        if(commandCode == 0x51)
        {      
            // printf("s t len\n");
            I2C_SlaveSendData(pI2CHandle->pI2Cx, strlen((char*)some_data));
        }
        else if (commandCode == 0x52)
		{
			//Send the contents of Tx_buf
            // printf("ac %d\n",cnt);
			I2C_SlaveSendData(pI2CHandle->pI2Cx, some_data[cnt++]);

		}
    }
    else if(AppEv == I2C_EV_TX_CMPLT)
    {
    	//  printf("Tx is completed\n");
    }
    else if(AppEv == I2C_EV_RX_CMPLT)
    {
        // printf("Rx is complete\n");
        // RxComplt = SET;
        commandCode = I2C_SlaveReceiveData(pI2CHandle->pI2Cx);
        // printf("a s r %d\n",commandCode);
        // if(commandCode == 0x52)
        // {      
        //     pI2CHandle->pI2Cx->SR2 |= (1<<I2C_SR2_TRA);
        //     // printf("slave trans call\n");
        //     I2C_SlaveSendData(pI2CHandle->pI2Cx, strlen((char*)some_data));
        // }
        // if(commandCode == 0x51)
        // {      
        //     pI2CHandle->pI2Cx->SR2 |= (1<<I2C_SR2_TRA);
            
        //     I2C_SlaveSendData(pI2CHandle->pI2Cx, 0x55);

        //     int len = strlen((char*)some_data);
        //     // while(len > 0)
        //     // {
        //     //     // printf("txe\n");
        //     //     //1. load the data in to DR
        //     //     uint8_t ch = some_data[0];
        //     //     printf("%d\n",ch);
        //     //     I2C_SlaveSendData(pI2CHandle->pI2Cx, ch);
        //     //     len--;
        //     // }
        //     // // pI2CHandle->pI2Cx->SR2 |= (1<<I2C_SR2_TRA);
        //     // printf("slave trans call\n");
            
        // }
        // printf("i2c1 %d\n",I2C1->DR);
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

#else
void I2C_ApplicationEventCallback(I2C_Handle_t *pI2CHandle,uint8_t AppEv)
{

	static uint8_t commandCode = 0;
	static  uint8_t Cnt = 0;

	if(AppEv == I2C_EV_DATA_REQ)
	{
		//Master wants some data. slave has to send it
		if(commandCode == 0x51)
		{
			//send the length information to the master
			I2C_SlaveSendData(pI2CHandle->pI2Cx,strlen((char*)Tx_buf));
		}
        else if (commandCode == 0x52)
		{
			//Send the contents of Tx_buf
			I2C_SlaveSendData(pI2CHandle->pI2Cx,Tx_buf[Cnt++]);

		}
	}else if (AppEv == I2C_EV_DATA_RCV)
	{
		//Data is waiting for the slave to read . slave has to read it
		commandCode = I2C_SlaveReceiveData(pI2CHandle->pI2Cx);

	}else if (AppEv == I2C_ERROR_AF)
	{
		//This happens only during slave txing .
		//Master has sent the NACK. so slave should understand that master doesnt need
		//more data.
		commandCode = 0xff;
		Cnt = 0;
	}
	else if (AppEv == I2C_EV_STOP)
	{
		//This happens only during slave reception .
		//Master has ended the I2C communication with the slave.
	}
}
#endif