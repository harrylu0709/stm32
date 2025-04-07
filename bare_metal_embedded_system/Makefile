CC=arm-none-eabi-gcc
OBJCOPY=arm-none-eabi-objcopy
MACH=cortex-m4
CFLAGS= -c -mcpu=$(MACH) -mthumb -mfloat-abi=soft -std=gnu11 -Wall -O0
LDFLAGS= -mcpu=$(MACH) -mthumb -mfloat-abi=soft --specs=nano.specs -T stm32_ls.ld -Wl,-Map=final.map
LDFLAGS_SH= -mcpu=$(MACH) -mthumb -mfloat-abi=soft --specs=rdimon.specs -T stm32_ls.ld -Wl,-Map=final.map

all:final.bin

semi:main.o led.o stm32_startup.o syscalls.o final_sh.elf

main.o:main.c
	$(CC) $(CFLAGS) -o $@ $^

led.o:led.c
	$(CC) $(CFLAGS) -o $@ $^

stm32_startup.o:stm32_startup.c
	$(CC) $(CFLAGS) -o $@ $^

syscalls.o:syscalls.c
	$(CC) $(CFLAGS) -o $@ $^

final.elf:main.o led.o stm32_startup.o syscalls.o
	$(CC) $(LDFLAGS) -o $@ $^

final.bin:final.elf
	$(OBJCOPY) -O binary $^ $@

final_sh.elf:main.o led.o stm32_startup.o
	$(CC) $(LDFLAGS_SH) -o $@ $^

clean:
	del -rf *.o *.elf *.bin *.map

load:
	openocd -f board/stm32f4discovery.cfg

program_elf:
	openocd -f board/stm32f4discovery.cfg -c "program final.elf verify reset exit"

program_bin:
	openocd -f board/stm32f4discovery.cfg -c "program final.bin verify reset exit 0x08000000"