CC=arm-none-eabi-gcc
MACH=cortex-m4
CFLAGS= -c -mcpu=$(MACH) -mthumb -mfloat-abi=soft -std=gnu11 -Wall -O0 -I./drivers/Inc -I./include
LDFLAGS= -mcpu=$(MACH) -mthumb -mfloat-abi=soft --specs=nano.specs -T stm32_ls.ld -Wl,-Map=final.map
LDFLAGS_SH= -mcpu=$(MACH) -mthumb -mfloat-abi=soft --specs=rdimon.specs -T stm32_ls.ld -Wl,-Map=final_sh.map
DRIVERS_DIR := drivers/Src
OBJ_FILES := $(wildcard $(DRIVERS_DIR)/Src/*.o)  # Adjust based on your structure
all:main.o lcd.o ds1307.o stm32_startup.o syscalls.o drivers/Src/stm32f407xx_gpio_driver.o drivers/Src/stm32f407xx_i2c_driver.o drivers/Src/stm32f407xx_tim_driver.o drivers/Src/stm32f407xx_rcc_driver.o final.elf

semi:main.o lcd.o ds1307.o stm32_startup.o syscalls.o drivers/Src/stm32f407xx_gpio_driver.o drivers/Src/stm32f407xx_i2c_driver.o drivers/Src/stm32f407xx_tim_driver.o drivers/Src/stm32f407xx_rcc_driver.o final_sh.elf

main.o:main.c 
	$(CC) $(CFLAGS) -o $@ $^

lcd.o:lcd.c
	$(CC) $(CFLAGS) -o $@ $^

ds1307.o:ds1307.c
	$(CC) $(CFLAGS) -o $@ $^

stm32_startup.o:stm32_startup.c
	$(CC) $(CFLAGS) -o $@ $^

syscalls.o:syscalls.c
	$(CC) $(CFLAGS) -o $@ $^

drivers/Src/stm32f407xx_gpio_driver.o:drivers/Src/stm32f407xx_gpio_driver.c
	$(CC) $(CFLAGS) -o $@ $^

drivers/Src/stm32f407xx_i2c_driver.o:drivers/Src/stm32f407xx_i2c_driver.c
	$(CC) $(CFLAGS) -o $@ $^

drivers/Src/stm32f407xx_tim_driver.o:drivers/Src/stm32f407xx_tim_driver.c	
	$(CC) $(CFLAGS) -o $@ $^

drivers/Src/stm32f407xx_rcc_driver.o:drivers/Src/stm32f407xx_rcc_driver.c	
	$(CC) $(CFLAGS) -o $@ $^

drivers/Src/stm32f407xx_spi_driver.o:drivers/Src/stm32f407xx_spi_driver.c	
	$(CC) $(CFLAGS) -o $@ $^

drivers/Src/stm32f407xx_usart_driver.o:drivers/Src/stm32f407xx_usart_driver.c	
	$(CC) $(CFLAGS) -o $@ $^

final.elf:main.o lcd.o ds1307.o stm32_startup.o syscalls.o drivers/Src/stm32f407xx_gpio_driver.o drivers/Src/stm32f407xx_i2c_driver.o drivers/Src/stm32f407xx_tim_driver.o drivers/Src/stm32f407xx_rcc_driver.o
	$(CC) $(LDFLAGS) -o $@ $^

final_sh.elf:main.o lcd.o ds1307.o stm32_startup.o drivers/Src/stm32f407xx_gpio_driver.o drivers/Src/stm32f407xx_i2c_driver.o drivers/Src/stm32f407xx_tim_driver.o drivers/Src/stm32f407xx_rcc_driver.o
	$(CC) $(LDFLAGS_SH) -o $@ $^

clean:
	rm -rf $(DRIVERS_DIR)/*.o
	rm -rf *.elf
	rm -rf *.o
	rm -rf *.map
load:
	openocd -f board/stm32f4discovery.cfg

program:
	openocd -f board/stm32f4discovery.cfg -c init -c halt -c "flash write_image erase final.elf" -c reset -c shutdown