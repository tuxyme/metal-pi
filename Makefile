

all:
	cd ./analyzer; make

install: all
	cp ./analyzer/kernel.img ./sdcard/

	
toolchain:
	wget -N https://github.com/raspberrypi/tools/archive/master.zip
	unzip master.zip
	
binaries:
	cd ./sdcard; wget -N --content-disposition https://github.com/raspberrypi/firmware/blob/master/boot/start.elf?raw=true
	cd ./sdcard; wget -N --content-disposition https://github.com/raspberrypi/firmware/blob/master/boot/bootcode.bin?raw=true
	
clean:
	cd ./analyzer; make clean
	cd ./common; make clean
	cd ./framebuffer; make clean
	cd ./interrupts; make clean
	cd ./uart; make clean
	




