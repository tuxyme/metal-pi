
ARMGNU ?= ../tools-master/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian/bin/arm-linux-gnueabihf
AFLAGS = --warn --fatal-warnings -mfpu=vfp -mfloat-abi=hard -march=armv6zk -mcpu=arm1176jzf-s
CFLAGS = -Wall -O2 -nostdlib -nostartfiles -ffreestanding -mfpu=vfp -mfloat-abi=hard -march=armv6zk -mtune=arm1176jzf-s
INCLUDES = -I../common/include -I../framebuffer

LIBS = \
../common/common.a \

all : depend kernel.img

depend:
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

uart.o : uart.c
	$(ARMGNU)-gcc $(CFLAGS) $(INCLUDES) -c uart.c -o uart.o

main.elf : memmap bootstrap.o uart.o main.o $(LIBS)
	$(ARMGNU)-ld bootstrap.o uart.o main.o $(LIBS) -T memmap -o main.elf

kernel.img : main.elf
	$(ARMGNU)-objcopy main.elf -O binary kernel.img
	$(ARMGNU)-objdump -D main.elf > main.dump






