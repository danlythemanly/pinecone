GCC=/usr/share/arduino/hardware/tools/avr/bin/avr-gcc 
OBJCOPY=/usr/share/arduino/hardware/tools/avr/bin/avr-objcopy
AVRDUDE=/usr/share/arduino/hardware/tools/avrdude

CFLAGS=-g -DF_CPU=8000000L -Wall -Os -Werror -Wextra -mmcu=attiny85
# for 1 MHz
#CFLAGS=-g -DF_CPU=1000000L -Wall -Os -Werror -Wextra -mmcu=attiny85
OFLAGS=-j .text -j .data -O ihex

all: hello.hex

hello.o: hello.c
	$(GCC) $(CFLAGS) -c -o hello.o hello.c

hello.elf: hello.o
	$(GCC) $(CFLAGS) -o hello.elf hello.o
	chmod a-x hello.elf

hello.hex: hello.elf
	$(OBJCOPY) $(OFLAGS) hello.elf hello.hex

fuses: 
	$(AVRDUDE) -C/home/djwillia/dev/arduino-1.6.4/hardware/tools/avr/etc/avrdude.conf -v -v -v -v -pattiny85 -cstk500v1 -P/dev/ttyACM0 -b19200 -e -Uefuse:w:0xff:m -Uhfuse:w:0xdf:m -Ulfuse:w:0xe2:m

# for 1 MHz
#    $(AVRDUDE) -C/home/djwillia/dev/arduino-1.6.4/hardware/tools/avr/etc/avrdude.conf -v -v -v -v -pattiny85 -cstk500v1 -P/dev/ttyACM0 -b19200 -e -Uefuse:w:0xff:m -Uhfuse:w:0xdf:m -Ulfuse:w:0x62:m


install: hello.hex
	stty -F /dev/ttyACM0 cs8 19200 ignbrk -brkint -icrnl -imaxbel -opost -onlcr -isig -icanon -iexten -echo -echoe -echok -echoctl -echoke noflsh -ixon -crtscts
	$(AVRDUDE) -C/home/djwillia/dev/arduino-1.6.4/hardware/tools/avr/etc/avrdude.conf -v -v -v -v -pattiny85 -cstk500v1 -P/dev/ttyACM0 -b19200 -U flash:w:hello.hex:i

clean:
	rm -f hello.elf hello.hex hello.o
