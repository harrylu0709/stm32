#include <stdint.h>
#include <string.h>
#include "ds1307.h"

#define I2C2_HANDLE    0

static void ds1307_i2c_pin_config(void);
static void ds1307_i2c_config(void);
static uint8_t ds1307_read(uint8_t reg_addr);
static void ds1307_write(uint8_t value,uint8_t reg_addr);
static uint8_t binary_to_bcd(uint8_t value);
static uint8_t bcd_to_binary(uint8_t value);
#if I2C_INT_ENABLE
uint8_t tx[2];
uint8_t ds1307_data;
uint8_t global_reg_addr;

#endif
I2C_Handle_t g_ds1307I2CHandle;

uint8_t ds1307_init(void)
{

	//1. init the i2c pins
	ds1307_i2c_pin_config();

	//2. initialize the i2c peripheral
	ds1307_i2c_config();

	//3. Enable the I2C peripheral
	I2C_PeripheralControl(DS1307_I2C, ENABLE);

	I2C_ManageAcking(DS1307_I2C, ENABLE);
	//4. Make clock halt = 0 
	ds1307_write(0x00, DS1307_ADDR_SEC);

	//5. Read back clock halt bit
	uint8_t clock_state = ds1307_read(DS1307_ADDR_SEC);

	return ((clock_state >> 7) & 0x1);
}


void ds1307_set_current_time(RTC_time_t *rtc_time)
{
	uint8_t seconds, hrs;
	seconds = binary_to_bcd(rtc_time->seconds);
	seconds &= ~(1<<7);
	ds1307_write(seconds, DS1307_ADDR_SEC);

	ds1307_write(binary_to_bcd(rtc_time->minutes),DS1307_ADDR_MIN);

	hrs = binary_to_bcd(rtc_time->hours);

	if(rtc_time->time_format == TIME_FORMAT_24HRS)
	{
		hrs &=(1<<6);
	}
	else
	{
		hrs |=(1<<6);
		hrs = (rtc_time->time_format == TIME_FORMAT_12HRS_PM) ? hrs | (1<<5) : hrs & ~(1<<5);
	}
	ds1307_write(hrs,DS1307_ADDR_HRS);

}
void ds1307_get_current_time(RTC_time_t *rtc_time)
{
	uint8_t seconds,hrs;

	seconds = ds1307_read(DS1307_ADDR_SEC);

	seconds &= ~( 1 << 7);

	rtc_time->seconds = bcd_to_binary(seconds);
	rtc_time->minutes = bcd_to_binary(ds1307_read(DS1307_ADDR_MIN));

	hrs = ds1307_read(DS1307_ADDR_HRS);
	if(hrs & ( 1 << 6)){
		//12 hr format
		rtc_time->time_format =  !((hrs & ( 1 << 5)) == 0) ;
		hrs &= ~(0x3 << 5);//Clear 6 and 5
	}else{
		//24 hr format
		rtc_time->time_format = TIME_FORMAT_24HRS;
	}

	rtc_time->hours = bcd_to_binary(hrs);
}

void ds1307_set_current_date(RTC_date_t *rtc_date)
{
	ds1307_write(binary_to_bcd(rtc_date->date),DS1307_ADDR_DATE);

	ds1307_write(binary_to_bcd(rtc_date->month),DS1307_ADDR_MONTH);

	ds1307_write(binary_to_bcd(rtc_date->year),DS1307_ADDR_YEAR);

	ds1307_write(binary_to_bcd(rtc_date->day),DS1307_ADDR_DAY);

}
void ds1307_get_current_date(RTC_date_t *rtc_date)
{
	rtc_date->date  = bcd_to_binary(ds1307_read(DS1307_ADDR_DATE));
	rtc_date->month = bcd_to_binary(ds1307_read(DS1307_ADDR_MONTH));
	rtc_date->year  = bcd_to_binary(ds1307_read(DS1307_ADDR_YEAR));
	rtc_date->day   = bcd_to_binary(ds1307_read(DS1307_ADDR_DAY));
}

static void ds1307_i2c_pin_config(void)
{
    GPIO_Handle_t   i2c_sda, i2c_scl;
    memset(&i2c_sda,0,sizeof(i2c_sda));
    memset(&i2c_scl,0,sizeof(i2c_scl));

    /*
    I2C_SCL => PB6
    I2C_SDA => PB7
    */

    i2c_scl.pGPIOx = DS1307_I2C_GPIO_PORT;
    i2c_scl.GPIO_PinConfig.GPIO_PinNumber = DS1307_I2C_SCL_PIN;
	i2c_scl.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_ALTFN;
	i2c_scl.GPIO_PinConfig.GPIO_PinAltFunMode = 4;
	i2c_scl.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_OD;
	i2c_scl.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;
	i2c_scl.GPIO_PinConfig.GPIO_PinPuPdControl = DS1307_I2C_PUPD;

	GPIO_Init(&i2c_scl);

    i2c_sda.pGPIOx = DS1307_I2C_GPIO_PORT;
    i2c_sda.GPIO_PinConfig.GPIO_PinNumber = DS1307_I2C_SDA_PIN;
	i2c_sda.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_ALTFN;
	i2c_sda.GPIO_PinConfig.GPIO_PinAltFunMode = 4;
	i2c_sda.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_OD;
	i2c_sda.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;
	i2c_sda.GPIO_PinConfig.GPIO_PinPuPdControl = DS1307_I2C_PUPD;

	GPIO_Init(&i2c_sda);
}

static void ds1307_i2c_config(void)
{
    g_ds1307I2CHandle.pI2Cx = DS1307_I2C;
    g_ds1307I2CHandle.I2C_Config.I2C_AckControl = I2C_ACK_ENABLE;
    g_ds1307I2CHandle.I2C_Config.I2C_SCLSpeed = DS1307_I2C_SPEED;
    g_ds1307I2CHandle.I2C_Config.I2C_FMDutyCycle = I2C_FM_DUTY_2;
    g_ds1307I2CHandle.I2C_Config.I2C_SCLSpeed = I2C_SCL_SPEED_SM;
    I2C_Init(&g_ds1307I2CHandle);
#if I2C_INT_ENABLE
    /* I2C IRQ configuration */
    I2C_IRQInterruptConfig(IRQ_NO_I2C1_EV, ENABLE);
    I2C_IRQInterruptConfig(IRQ_NO_I2C1_ER, ENABLE);
    //I2C_PeripheralControl(DS1307_I2C, ENABLE);
#endif
}

static void ds1307_write(uint8_t value, uint8_t reg_addr)
{
#if I2C_INT_ENABLE
	tx[0] = reg_addr;
	tx[1] = value;
	while(I2C_MasterSendDataIT_one(&g_ds1307I2CHandle, tx, 2, DS1307_I2C_ADDRESS, I2C_DISABLE_SR) != 0);
	g_ds1307I2CHandle.TxRxComplt = RESET;

	// //wait till tx completes
	while(g_ds1307I2CHandle.TxRxComplt != SET);

	g_ds1307I2CHandle.TxRxComplt = RESET;
#else
	uint8_t tx[2];
	tx[0] = reg_addr;
	tx[1] = value;
	I2C_MasterSendData(&g_ds1307I2CHandle, tx, 2, DS1307_I2C_ADDRESS, 0, 0);
#endif
}

static uint8_t ds1307_read(uint8_t reg_addr)
{
#if I2C_INT_ENABLE
	global_reg_addr = reg_addr;

	while(I2C_MasterSendDataIT_one(&g_ds1307I2CHandle, &global_reg_addr, 1,DS1307_I2C_ADDRESS, I2C_DISABLE_SR) != 0);

	while(I2C_MasterReceiveDataIT(&g_ds1307I2CHandle, &ds1307_data, 1, DS1307_I2C_ADDRESS, I2C_DISABLE_SR)!= 0);
	
	g_ds1307I2CHandle.TxRxComplt = RESET;

	// //wait till rx completes
	while(g_ds1307I2CHandle.TxRxComplt != SET);

	g_ds1307I2CHandle.TxRxComplt = RESET;

#else
	uint8_t ds1307_data;
	I2C_MasterSendData(&g_ds1307I2CHandle, &reg_addr, 1, DS1307_I2C_ADDRESS, 0, 0);
	//I2C_MasterSendData(&g_ds1307I2CHandle, &reg_addr, 1, DS1307_I2C_ADDRESS, I2C_DISABLE_SR);
	I2C_MasterReceiveData(&g_ds1307I2CHandle, &ds1307_data, 1, DS1307_I2C_ADDRESS, 0);
#endif
	return ds1307_data;
}

static uint8_t binary_to_bcd(uint8_t value)
{
	uint8_t m,n;
	uint8_t bcd;

	bcd = value;
	if(value >= 10)
	{
		m = value / 10;
		n = value % 10;
		bcd = (m<<4)|n;
	}
	
	return bcd;
}

static uint8_t bcd_to_binary(uint8_t value)
{
	uint8_t m , n;
	m = (uint8_t) ((value >> 4 ) * 10);
	n =  value & (uint8_t)0x0F;
	return (m+n);
}

