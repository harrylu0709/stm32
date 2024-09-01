#include<stdio.h>
#include<string.h>
#include "stm32f407xx.h"

/*
    USART1
    TX:PB6
    RX:PB7

    USART2
    TX:PA2
    RX:PA3
*/
char msg[] = "uart test\n";
// uint8_t msg[] = {0x11,0x22,0x33};
uint8_t rcv_buf[32];

USART_Handle_t usart1_handle;
USART_Handle_t usart2_handle;


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

void USART1_Init(void)
{// USART_MODE_TXRX
    usart1_handle.pUSARTx = USART1;
    usart1_handle.USART_Config.USART_Baud = USART_STD_BAUD_115200;
    usart1_handle.USART_Config.USART_HWFlowControl = USART_HW_FLOW_CTRL_NONE;
    usart1_handle.USART_Config.USART_Mode = USART_MODE_TXRX;//USART_MODE_ONLY_TX;
    usart1_handle.USART_Config.USART_NoOfStopBits = USART_STOPBITS_1;
    usart1_handle.USART_Config.USART_ParityControl = USART_PARITY_DISABLE;
    USART_Init(&usart1_handle);
}

void USART2_Init(void)
{
    usart2_handle.pUSARTx = USART2;
    usart2_handle.USART_Config.USART_Baud = USART_STD_BAUD_115200;
    usart2_handle.USART_Config.USART_HWFlowControl = USART_HW_FLOW_CTRL_NONE;
    usart2_handle.USART_Config.USART_Mode = USART_MODE_TXRX;//USART_MODE_ONLY_RX;
    usart2_handle.USART_Config.USART_NoOfStopBits = USART_STOPBITS_1;
    usart2_handle.USART_Config.USART_ParityControl = USART_PARITY_DISABLE;
    USART_Init(&usart2_handle);
}

void USART1_GPIOInit(void)
{
	GPIO_Handle_t USART1Pins;

    USART1Pins.pGPIOx = GPIOB;
    USART1Pins.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_ALTFN;
    USART1Pins.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_PP;
    USART1Pins.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_PIN_PU;
    USART1Pins.GPIO_PinConfig.GPIO_PinAltFunMode = 7;
    USART1Pins.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;

    //TX
    USART1Pins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_6;
    GPIO_Init(&USART1Pins);

    //RX
    USART1Pins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_7;
    GPIO_Init(&USART1Pins);
}

void USART2_GPIOInit(void)
{
	GPIO_Handle_t USART2Pins;

    USART2Pins.pGPIOx = GPIOA;
    USART2Pins.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_ALTFN;
    USART2Pins.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_PP;
    USART2Pins.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_PIN_PU;
    USART2Pins.GPIO_PinConfig.GPIO_PinAltFunMode = 7;
    USART2Pins.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;

    //TX
    USART2Pins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_2;
    GPIO_Init(&USART2Pins);

    //RX
    USART2Pins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_3;
    GPIO_Init(&USART2Pins);
}


int main(void)
{
    GPIO_ButtonInit();

    USART1_GPIOInit();
    USART1_Init();

    USART2_GPIOInit();
    USART2_Init();

    USART_IRQInterruptConfig(IRQ_NO_USART1,ENABLE);
    USART_IRQInterruptConfig(IRQ_NO_USART2,ENABLE);

    USART_PeripheralControl(USART1, ENABLE);
    USART_PeripheralControl(USART2, ENABLE);

    
    while(1)
    {
    //		//wait till button is pressed
        while( ! GPIO_ReadFromInputPin(GPIOA, GPIO_PIN_NO_0) );

        //to avoid button de-bouncing related issues 200ms of delay
        delay();

        USART_SendDataIT(&usart1_handle, (uint8_t*)msg, strlen(msg));
        USART_ReceiveDataIT(&usart2_handle, rcv_buf, 32);
        // USART_SendData(&usart1_handle, msg, 3);
        // USART_SendData(&usart2_handle, rcv_buf, 32);        
    }
    return 0;
}

void USART1_IRQHandler(void)
{   
    USART_IRQHandling(&usart1_handle);
}

void USART2_IRQHandler(void)
{   
    USART_IRQHandling(&usart2_handle);
}

void USART_ApplicationEventCallback(USART_Handle_t *pUSARTHandle,uint8_t AppEv)
{

}
