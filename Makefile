CC = xtensa-lx106-elf-gcc
CFLAGS = -Os -I. -mlongcalls
LDLIBS = -nostdlib -Wl,--start-group -lcirom -lpwm -lmain -lnet80211 -lwpa -llwip -lpp -lphy -Wl,--end-group -lgcc
LDFLAGS = -Teagle.app.v6.ld

traktor-0x00000.bin: traktor
	esptool.py elf2image $^

traktor: traktor.o

traktor.o: traktor.c

flash: traktor-0x00000.bin
	esptool.py write_flash 0 traktor-0x00000.bin 0x10000 traktor-0x10000.bin

clean:
	rm -f traktor traktor.o traktor-0x00000.bin traktor-0x10000.bin
