
ARMGNU ?= ../tools-master/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian/bin/arm-linux-gnueabihf
AFLAGS = --warn --fatal-warnings -mfpu=vfp -mfloat-abi=hard -march=armv6zk -mcpu=arm1176jzf-s
CFLAGS = -Wall -O2 -nostdlib -nostartfiles -ffreestanding -mfpu=vfp -mfloat-abi=hard -march=armv6zk -mtune=arm1176jzf-s
INCLUDES = -I../common/include -I../framebuffer -I../uart

LIBS = \
../uart/uart.o \
../framebuffer/framebuffer.a \
../common/common.a \

all : depend kernel.img

depend:
	cd ../uart; make
	cd ../framebuffer; make
	cd ../common; make

clean :
	rm -f *.o
	rm -f *.bin
	rm -f *.img
	rm -f *.elf
	rm -f *.dump

bootstrap.o : bootstrap.s
	$(ARMGNU)-as $(AFLAGS) bootstrap.s -o bootstrap.o

main.o : main.c
	$(ARMGNU)-gcc $(CFLAGS) $(INCLUDES) -c main.c -o main.o

sampler.o : sampler.c
	$(ARMGNU)-gcc $(CFLAGS) $(INCLUDES) -c sampler.c -o sampler.o

main.elf : memmap.ld bootstrap.o main.o sampler.o $(LIBS)
	$(ARMGNU)-ld bootstrap.o main.o sampler.o $(LIBS) -T memmap.ld -o main.elf

kernel.img : main.elf
	$(ARMGNU)-objcopy main.elf -O binary kernel.img
	$(ARMGNU)-objdump -D main.elf > main.dump






