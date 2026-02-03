# Which QEMU to use
QEMU=qemu-system-arm

# what toolchain to use
TOOLCHAIN=arm-none-eabi

# name of the board
BOARD=versatile

# Number of KB to be used, try first with 16,
# later on, you will need more, but less than 1024.
MEMSIZE=16

# Object files to build and link together
objs= exception.o startup.o irq.o main.o uart.o timer.o isr.o

#======================================================================
# GENERIC PART OF THE MAKEFILE BELOW
# ONLY CONFIGURE VARIABLES ABOVE.
#======================================================================
.PHONY: all build clean clean-all run debug

ifeq ($(BOARD),versatile)
  # set the processor type
  CPU=CORTEX_A8
	GCPU=cortex-a8
	QCPU=cortex-a8
  # A few QEMU options
  VGA=-nographic
  SERIAL=-serial mon:stdio  
  MEMORY="$(MEMSIZE)K"  
  MACHINE=versatilepb
	# set compiler flags
  CFLAGS= -mcpu=$(GCPU) -DCPU=$(QCPU) -D$(CPU) -DMEMORY="($(MEMSIZE)*1024)"
  CFLAGS+= -c -g -nostdlib -ffreestanding \
         -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wundef
	# set assembler flags
  ASFLAGS= -mcpu=$(GCPU) -g
  # set linker flags, also specifying the linker script file
  LDFLAGS= -g -T versatile.ld -nostdlib -static
endif

# Check that the given BOARD was recognized 
# and consequently the MACHINE was set.
ifndef MACHINE
  $(error Must choose a board (e.g. Versatile AB or PB)) 
endif

#-------------------------------------------------------------
# Build Part
#-------------------------------------------------------------

# Board-dependent BUILD directory
# to allow to build for different target boards.
BUILD=build/$(BOARD)

# Ask GCC to produce accurate dependencies
CFLAGS+=-MT $@ -MMD -MP -MF $(BUILD)/$*.d
	
OBJS = $(addprefix $(BUILD)/, $(objs)) 

# Compilation Rules
$(BUILD)/%.o: %.c
	$(TOOLCHAIN)-gcc $(CFLAGS) -o $@ $<

$(BUILD)/%.o: %.s
	$(TOOLCHAIN)-as $(ASFLAGS) -o $@ $<

$(BUILD)/%.o: %.S
	$(TOOLCHAIN)-gcc $(CFLAGS) -o $@ $<

# Build and link all
# Notice that we link with our own linker script: test.ld
all: build $(OBJS)
	$(TOOLCHAIN)-ld $(LDFLAGS) $(OBJS) -o $(BUILD)/kernel.elf
	$(TOOLCHAIN)-objcopy -O binary $(BUILD)/kernel.elf $(BUILD)/kernel.bin 

build:
	@mkdir -p $(BUILD)
	@mkdir -p $(BUILD)/memory 

# Include all the gcc-generated dependencies
-include $(wildcard $(BUILD)/*.d)

#-------------------------------------------------------------
# Clean Part
#-------------------------------------------------------------
clean: 
	rm -rf $(BUILD)/

clean-all: 
	rm -rf build/
	
#-------------------------------------------------------------
# Execution Part
#-------------------------------------------------------------
ifeq ($(BOARD),versatile)
run: all
	@echo "\n\nBoard: Versatile Board...\n"
	$(QEMU) -M $(MACHINE) -cpu $(QCPU) -m $(MEMORY) $(VGA) $(SERIAL) -device loader,file=$(BUILD)/kernel.elf

debug: all
	@echo "\n\nBoard: Versatile Board...\n"
	$(QEMU) -M $(MACHINE) -cpu $(QCPU) -m $(MEMORY) $(VGA) $(SERIAL) -device loader,file=$(BUILD)/kernel.elf -gdb tcp::1234 -S
# -cpu $(CPU)

endif
