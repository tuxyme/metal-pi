
ARMGNU ?= ../tools-master/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian/bin/arm-linux-gnueabihf
AFLAGS = --warn --fatal-warnings -mfpu=vfp -mfloat-abi=hard -march=armv6zk -mcpu=arm1176jzf-s
CFLAGS = -Wall -O2 -nostdlib -nostartfiles -ffreestanding -mfpu=vfp -mfloat-abi=hard -march=armv6zk -mtune=arm1176jzf-s

LIBS = ../uart/uart.o

all: common.a

clean :
	rm -f *.o
	rm -f *.a

div.o : div.s
	$(ARMGNU)-as $(AFLAGS) div.s -o div.o

misc.o : misc.s
	$(ARMGNU)-as $(AFLAGS) misc.s -o misc.o

gpio.o : gpio.c
	$(ARMGNU)-gcc $(CFLAGS) -c gpio.c -o gpio.o

string.o : string.c
	$(ARMGNU)-gcc $(CFLAGS) -c string.c -o string.o

mailbox.o : mailbox.c
	$(ARMGNU)-gcc $(CFLAGS) -c mailbox.c -o mailbox.o

utils.o : utils.c
	$(ARMGNU)-gcc $(CFLAGS) -c utils.c -o utils.o

common.a: gpio.o string.o utils.o div.o misc.o mailbox.o
	$(ARMGNU)-ar rcs common.a gpio.o string.o utils.o div.o misc.o mailbox.o

	
