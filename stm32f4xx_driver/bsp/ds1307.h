#ifndef DS_1307_H
#define DS_1307_H
#include "stm32f407xx.h"

#define DS1307_I2C              I2C1
#define DS1307_I2C_GPIO_PORT    GPIOB
#define DS1307_I2C_SDA_PIN      GPIO_PIN_NO_7
#define DS1307_I2C_SCL_PIN      GPIO_PIN_NO_6
#define DS1307_I2C_SPEED        I2C_SCL_SPEED_SM
#define DS1307_I2C_PUPD         GPIO_PIN_PU


#define DS1307_ADDR_SEC         0
#define DS1307_ADDR_MIN         1
#define DS1307_ADDR_HRS         2
#define DS1307_ADDR_DAY         3
#define DS1307_ADDR_DATE        4
#define DS1307_ADDR_MONTH       5
#define DS1307_ADDR_YEAR        6

#define TIME_FORMAT_12HRS_AM    0
#define TIME_FORMAT_12HRS_PM    1
#define TIME_FORMAT_24HRS       2

#define DS1307_I2C_ADDRESS      0x68
#define SUNDAY                  0x00
#define MODAY                   0x01
#define TUESDAY                 0x02
#define WEDNESDAY               0x03
#define THURSDAY                0x04
#define FRIDAY                  0x05
#define SATURDAY                0x06

typedef struct 
{
    uint8_t date;
    uint8_t month;
    uint8_t year;
    uint8_t day;
}RTC_date_t;

typedef struct 
{
    uint8_t seconds;
    uint8_t minutes;
    uint8_t hours;
    uint8_t time_format;
}RTC_time_t;

uint8_t ds1307_init(void);
void ds1307_set_current_time(RTC_time_t *);
void ds1307_get_current_time(RTC_time_t *);

void ds1307_set_current_date(RTC_date_t *);
void ds1307_get_current_date(RTC_date_t *);
#endif