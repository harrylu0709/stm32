

#include<stdio.h>
#include "ds1307.h"

#include "lcd.h"

#define SYSTICK_TIM_CLK   16000000UL //16MHz
extern I2C_Handle_t g_ds1307I2CHandle;
TIM_Handle_t Timer5_Handle;

/* Enable this macro if you want to test RTC on LCD */
//#define PRINT_LCD //If LCD cannot display anything, it is necessary to connect VE(LCD contrast) to ground for the board

/*
 * Embedded Systems Programming on ARM Cortex-M3/M4 Processor lecture 79

 * Section 4.4 of Cortex -M4 Devices Generic User Guide
*/
#if ENABLE_SysTick
void init_systick_timer(uint32_t tick_hz)
{
	uint32_t *pSRVR = (uint32_t*)0xE000E014;
	uint32_t *pSCSR = (uint32_t*)0xE000E010;

    /* calculation of reload value */
    uint32_t count_value = (SYSTICK_TIM_CLK/tick_hz)-1;// if 1 second, then tick_hz = 1, if 1 millisecond, then tick_hz = 1000

    //Clear the value of SVR
    *pSRVR &= ~(0x00FFFFFFFF);

    //load the value in to SVR
    *pSRVR |= count_value;

    //do some settings
    *pSCSR |= ( 1 << 1); //Enables SysTick exception request:

    *pSCSR |= ( 1 << 2);  //Indicates the clock source, processor clock source

    //enable the systick
    *pSCSR |= ( 1 << 0); //enables the counter

}
#endif
void close_systick_timer(void)
{
	uint32_t *pSRVR = (uint32_t*)0xE000E014;
	uint32_t *pSCSR = (uint32_t*)0xE000E010;

    //Clear the value of SVR
    *pSRVR &= ~(0x00FFFFFFFF);


    //do some settings
    *pSCSR &= ~(0x07);
}

void Systick_PriorityConfig(uint32_t IRQPriority)
{
	uint32_t *pSystick_Prioirty = (uint32_t*)0xE000ED20;

	*pSystick_Prioirty  |=  ( IRQPriority << 24);

}

char* get_day_of_week(uint8_t i)
{
	char* days[] = { "Sunday","Monday","Tuesday","Wednesday","Thursday","Friday","Saturday"};

	return days[i-1];
}


void number_to_string(uint8_t num , char* buf)
{

	if(num < 10){
		buf[0] = '0';
		buf[1] = num+48;
	}else if(num >= 10 && num < 99)
	{
		buf[0] = (num/10) + 48;
		buf[1]= (num % 10) + 48;
	}
}



//hh:mm:ss
char* time_to_string(RTC_time_t *rtc_time)
{
	static char buf[9];

	buf[2]= ':';
	buf[5]= ':';

	number_to_string(rtc_time->hours,buf);
	number_to_string(rtc_time->minutes,&buf[3]);
	number_to_string(rtc_time->seconds,&buf[6]);

	buf[8] = '\0';

	return buf;

}

//dd/mm/yy
char* date_to_string(RTC_date_t *rtc_date)
{
	static char buf[9];

	buf[2]= '/';
	buf[5]= '/';

	number_to_string(rtc_date->date,buf);
	number_to_string(rtc_date->month,&buf[3]);
	number_to_string(rtc_date->year,&buf[6]);

	buf[8]= '\0';

	return buf;

}

void TIM5_init(void)
{
	Timer5_Handle.pTIMx = TIM5;
	Timer5_Handle.Prescalar = 15999;
	Timer5_Handle.Reload_Val = 1000;
	TIM_Init(&Timer5_Handle);
}	

void delay(void)
{
	for(uint32_t i = 0 ; i < 599000/2 ; i ++);
}

int main(void)
{

	RTC_time_t current_time;
	RTC_date_t current_date;

#ifndef PRINT_LCD
	printf("RTC test\n");
#else
	lcd_init();

	lcd_print_string("RTC Test...");

	lcd_display_clear();
	lcd_display_return_home();
#endif

	if(ds1307_init()){
		printf("RTC init has failed\n");
		while(1);
	}



	current_date.day = FRIDAY;
	current_date.date = 15;
	current_date.month = 1;
	current_date.year = 21;

	current_time.hours = 11;
	current_time.minutes = 59;
	current_time.seconds = 50;
	current_time.time_format = TIME_FORMAT_12HRS_PM;
	
	 ds1307_set_current_date(&current_date);
	 ds1307_set_current_time(&current_time);

	 ds1307_get_current_time(&current_time);
	 ds1307_get_current_date(&current_date);
#if ENABLE_SysTick
	init_systick_timer(1);
#endif
	char *am_pm;
	if(current_time.time_format != TIME_FORMAT_24HRS){
		am_pm = (current_time.time_format) ? "PM" : "AM";
#ifndef PRINT_LCD
		printf("Current time = %s %s\n",time_to_string(&current_time),am_pm); // 04:25:41 PM
#else
		lcd_print_string(time_to_string(&current_time));
		lcd_print_string(am_pm);
#endif
	}else{
#ifndef PRINT_LCD
		printf("Current time = %s\n",time_to_string(&current_time)); // 04:25:41
#else
		lcd_print_string(time_to_string(&current_time));
#endif
	}

#ifndef PRINT_LCD
	printf("Current date = %s <%s>\n",date_to_string(&current_date), get_day_of_week(current_date.day));
#else
	lcd_set_cursor(2, 1);
	lcd_print_string(date_to_string(&current_date));
	lcd_print_char('<');
	lcd_print_string(get_day_of_week(current_date.day));
	lcd_print_char('>');
#endif

	
	TIM5_init();

	I2C_IRQInterruptConfig(IRQ_NO_I2C1_EV, ENABLE);
	
	TIM_IRQPriorityConfig(IRQ_NO_TIM5, NVIC_IRQ_PRI15);
	I2C_IRQPriorityConfig(IRQ_NO_I2C1_EV, NVIC_IRQ_PRI0);


	while(1);

	return 0;
}

void TIM5_IRQHandler(void)
{
	//printf("TIM5\n");
	/* Add a little delay to wait until I2C finish receive data by interrupt */
	delay(); 

	RTC_time_t current_time;
	RTC_date_t current_date;

	ds1307_get_current_time(&current_time);

	char *am_pm;
	if(current_time.time_format != TIME_FORMAT_24HRS){
		am_pm = (current_time.time_format) ? "PM" : "AM";
#ifndef PRINT_LCD
		printf("Current time = %s %s\n",time_to_string(&current_time),am_pm); // 04:25:41 PM
#else
		lcd_set_cursor(1, 1);
		lcd_print_string(time_to_string(&current_time));
		lcd_print_string(am_pm);
#endif

	}else{
#ifndef PRINT_LCD
		printf("Current time = %s\n",time_to_string(&current_time)); // 04:25:41
#else
		lcd_set_cursor(1, 1);
		lcd_print_string(time_to_string(&current_time));
#endif
	}

	ds1307_get_current_date(&current_date);

#ifndef PRINT_LCD
	printf("Current date = %s <%s>\n",date_to_string(&current_date), get_day_of_week(current_date.day));
#else
	lcd_set_cursor(2, 1);
	lcd_print_string(date_to_string(&current_date));
	lcd_print_char('<');
	lcd_print_string(get_day_of_week(current_date.day));
	lcd_print_char('>');
#endif
	Timer5_Handle.pTIMx->SR &= ~(1<<0);
}


#if I2C_INT_ENABLE
void I2C1_EV_IRQHandler(void)
{
    //printf("i2c1 ev\n");
	//printf("Handler %d\n",*(g_ds1307I2CHandle.pTxBuffer));
    I2C_EV_IRQHandling(&g_ds1307I2CHandle);
}

void I2C1_ER_IRQHandler(void)
{
    //printf("i2c1 er\n");
    I2C_ER_IRQHandling(&g_ds1307I2CHandle);
}

void I2C_ApplicationEventCallback(I2C_Handle_t *pI2CHandle,uint8_t AppEv)
{
    if(AppEv == I2C_EV_TX_CMPLT)
    {
        //printf("Tx is complete\n");
		pI2CHandle->TxRxComplt = SET;
    }
    else if(AppEv == I2C_EV_RX_CMPLT)
    {
        //printf("Rx is complete\n");
        pI2CHandle->TxRxComplt = SET;
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
    else if(AppEv == I2C_ERROR_BERR)
    {
        printf("Error: bus error\n");
    }
    else if(AppEv == I2C_ERROR_ARLO)
    {
        printf("Error: arb error\n");
    }
    else if(AppEv == I2C_ERROR_OVR)
    {
        printf("Error: overrun error\n");
    }
    else if(AppEv == I2C_ERROR_TIMEOUT)
    {
        printf("Error: timeout error\n");
    }

}
#endif

#if ENABLE_SysTick
void SysTick_Handler(void)
{
	RTC_time_t current_time;
	RTC_date_t current_date;

	ds1307_get_current_time(&current_time);

	char *am_pm;
	if(current_time.time_format != TIME_FORMAT_24HRS){
		am_pm = (current_time.time_format) ? "PM" : "AM";
#ifndef PRINT_LCD
		printf("Current time = %s %s\n",time_to_string(&current_time),am_pm); // 04:25:41 PM
#else
		lcd_set_cursor(1, 1);
		lcd_print_string(time_to_string(&current_time));
		lcd_print_string(am_pm);
#endif

	}else{
#ifndef PRINT_LCD
		printf("Current time = %s\n",time_to_string(&current_time)); // 04:25:41
#else
		lcd_set_cursor(1, 1);
		lcd_print_string(time_to_string(&current_time));
#endif
	}

	ds1307_get_current_date(&current_date);

#ifndef PRINT_LCD
	printf("Current date = %s <%s>\n",date_to_string(&current_date), get_day_of_week(current_date.day));
#else
	lcd_set_cursor(2, 1);
	lcd_print_string(date_to_string(&current_date));
	lcd_print_char('<');
	lcd_print_string(get_day_of_week(current_date.day));
	lcd_print_char('>');
#endif
}
#endif
