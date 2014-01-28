
ARMGNU ?= ../tools-master/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian/bin/arm-linux-gnueabihf
AFLAGS = --warn --fatal-warnings -mfpu=vfp -mfloat-abi=hard -march=armv6zk -mcpu=arm1176jzf-s
CFLAGS = -Wall -O2 -nostdlib -nostartfiles -ffreestanding -mfpu=vfp -mfloat-abi=hard -march=armv6zk -mtune=arm1176jzf-s
INCLUDES = -I../common/include -I../uart

#tuxy_250x280.o

OBJS=\
tuxy_250x280.o \
ter-u16n.o     \
ter-u22n.o     \
bootstrap.o    \
psf.o          \
main.o         \
framebuffer.o  \
../uart/uart.o \
../common/common.a \
../interrupts/interrupt.o \

EXT_DIR=libs

all: depend kernel.img framebuffer.a

depend:
	cd ../common; make
	
kernel.elf: memmap.ld $(OBJS)
	$(ARMGNU)-ld $(OBJS) -T memmap.ld -o $@

kernel.img: kernel.elf
	$(ARMGNU)-objcopy kernel.elf -O binary kernel.img
	$(ARMGNU)-objdump -D kernel.elf > kernel.dump

clean:
	$(RM) -f *.elf
	$(RM) -f *.img
	$(RM) -f *.d
	$(RM) -f *.o
	$(RM) -f *.a
	$(RM) -f *.dump

%.o: %.c Makefile
	$(ARMGNU)-gcc $(CFLAGS) $(INCLUDES) -c $< -o $@

%.o: %.s Makefile
	$(ARMGNU)-gcc $(CFLAGS) -c $< -o $@
	
%.o: $(EXT_DIR)/%.psfu
	$(ARMGNU)-objcopy -I binary -O elf32-littlearm -B arm $< $@

%.o: $(EXT_DIR)/%.rgb
	$(ARMGNU)-objcopy -I binary -O elf32-littlearm -B arm $< $@

log.o: ../common/log.c
	$(ARMGNU)-gcc $(CFLAGS) -c ../common/log.c -o ../common/log.c

framebuffer.a: framebuffer.o psf.o ter-u16n.o
	$(ARMGNU)-ar rcs framebuffer.a framebuffer.o ter-u16n.o psf.o
