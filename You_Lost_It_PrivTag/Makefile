CC=arm-none-eabi-gcc
OBJCOPY=arm-none-eabi-objcopy
MACH=cortex-m4
CFLAGS= -c -mcpu=$(MACH) -mthumb -mfloat-abi=soft -std=gnu11 -Wall -O0 -I./drivers/Inc -I./include
LDFLAGS= -mcpu=$(MACH) -mthumb -mfloat-abi=soft --specs=nano.specs -T stm32_ls.ld -Wl,-Map=final.map
LDFLAGS_SH= -mcpu=$(MACH) -mthumb -mfloat-abi=soft --specs=rdimon.specs -T stm32_ls.ld -Wl,-Map=final_sh.map
# Directories
SRC_DIR = .
DRIVERS_DIR = drivers/Src
# Find all .c files
SRCS = $(wildcard $(SRC_DIR)/*.c) $(wildcard $(DRIVERS_DIR)/*.c)
# Convert .c to .o
OBJS = $(SRCS:.c=.o)
# Output binary
TARGET_NAME = final
ELF = $(TARGET_NAME).elf
ELF_SH = final_sh.elf
BIN = $(TARGET_NAME).bin

all: $(BIN)
semi: $(ELF_SH)
# Compile .c to .o
%.o: %.c
	$(CC) $(CFLAGS) -o $@ $^

# Build the target
$(ELF):$(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

# Build the target
$(ELF_SH):$(filter-out ./syscalls.o, $(OBJS))
	$(CC) $(LDFLAGS_SH) -o $@ $^

# Convert elf to bin
$(BIN): $(ELF)
	$(OBJCOPY) -O binary $^ $@

clean:
	rm -rf $(DRIVERS_DIR)/*.o
	rm -rf *.elf
	rm -rf *.o
	rm -rf *.map
	rm -rf *.bin
load:
	openocd -f board/stm32f4discovery.cfg

program_elf:
	openocd -f board/stm32f4discovery.cfg -c init -c halt -c "flash write_image erase $(ELF)" -c reset -c shutdown

elf_sh:
	openocd -f board/stm32f4discovery.cfg -c init -c halt -c "flash write_image erase $(ELF_SH)" -c "arm semihosting enable" -c reset

program_bin:
	openocd -f board/stm32f4discovery.cfg -c "program $(BIN) verify reset exit 0x08000000"
#openocd -f board/stm32f4discovery.cfg -c "program final.elf verify reset exit"
#openocd -f board/stm32f4discovery.cfg -c "program final.bin exit 0x08000000"
#https://openocd.org/doc/html/Flash-Programming.html