GCC=/usr/share/arduino/hardware/tools/avr/bin/avr-gcc 
OBJCOPY=/usr/share/arduino/hardware/tools/avr/bin/avr-objcopy
AVRDUDE=/usr/share/arduino/hardware/tools/avrdude

# for 8 MHz
CFLAGS=-g -DF_CPU=8000000L -Wall -Os -Werror -Wextra -mmcu=attiny85
#CFLAGS=-g -Os -w -fno-exceptions -ffunction-sections -fdata-sections -fno-threadsafe-statics -MMD -mmcu=attiny85 -DF_CPU=8000000L -DARDUINO=10604 -DARDUINO_attiny -DARDUINO_ARCH_AVR -I/home/djwillia/dev/arduino-1.6.4/hardware/arduino/avr/cores/arduino -I/home/djwillia/.arduino15/packages/attiny/hardware/avr/1.0.1/variants/tiny8 


# for 1 MHz
#CFLAGS=-g -DF_CPU=1000000L -Wall -Os -Werror -Wextra -mmcu=attiny85
#CFLAGS=-g -DF_CPU=1000000L -Wall -Os -Wextra -mmcu=attiny85
OFLAGS=-j .text -j .data -O ihex
#OFLAGS_EEP=-O ihex -j .eeprom --set-section-flags=.eeprom=alloc,load --no-change-warnings --change-section-lma .eeprom=0 
#OFLAGS_HEX=-O ihex -R .eeprom 

all: audio_sample.hex

%.o : %.c
	$(GCC) $(CFLAGS) -c -o $@ $<

%.elf : %.o
	$(GCC) $(CFLAGS) -o $@ $<
	chmod a-x $@


%.hex : %.elf
	$(OBJCOPY) $(OFLAGS) $< $@

# hello.o: hello.c
# 	$(GCC) $(CFLAGS) -c -o hello.o hello.c

# hello.elf: hello.o
# 	$(GCC) $(CFLAGS) -o hello.elf hello.o
# 	chmod a-x hello.elf

# hello.hex: hello.elf
# 	$(OBJCOPY) $(OFLAGS) hello.elf hello.hex

# fuses: 
# 	$(AVRDUDE) -C/home/djwillia/dev/arduino-1.6.4/hardware/tools/avr/etc/avrdude.conf -v -v -v -v -pattiny85 -cstk500v1 -P/dev/ttyACM0 -b19200 -e -Uefuse:w:0xff:m -Uhfuse:w:0xdf:m -Ulfuse:w:0x62:m


# for 8 MHz
fuses:
	$(AVRDUDE) -C/home/djwillia/dev/arduino-1.6.4/hardware/tools/avr/etc/avrdude.conf -v -v -v -v -pattiny85 -cstk500v1 -P/dev/ttyACM0 -b19200 -e -Uefuse:w:0xff:m -Uhfuse:w:0xdf:m -Ulfuse:w:0xe2:m


install: hello.hex
	stty -F /dev/ttyACM0 cs8 19200 ignbrk -brkint -icrnl -imaxbel -opost -onlcr -isig -icanon -iexten -echo -echoe -echok -echoctl -echoke noflsh -ixon -crtscts
	$(AVRDUDE) -C/home/djwillia/dev/arduino-1.6.4/hardware/tools/avr/etc/avrdude.conf -v -v -v -v -pattiny85 -cstk500v1 -P/dev/ttyACM0 -b19200 -U flash:w:hello.hex:i

install_audio: audio_sample.hex
	stty -F /dev/ttyACM0 cs8 19200 ignbrk -brkint -icrnl -imaxbel -opost -onlcr -isig -icanon -iexten -echo -echoe -echok -echoctl -echoke noflsh -ixon -crtscts
	$(AVRDUDE) -C/home/djwillia/dev/arduino-1.6.4/hardware/tools/avr/etc/avrdude.conf -v -pattiny85 -cstk500v1 -P/dev/ttyACM0 -b19200 -U flash:w:audio_sample.hex:i

install_p2: pinecone2.hex
	stty -F /dev/ttyACM0 cs8 19200 ignbrk -brkint -icrnl -imaxbel -opost -onlcr -isig -icanon -iexten -echo -echoe -echok -echoctl -echoke noflsh -ixon -crtscts
	$(AVRDUDE) -C/home/djwillia/dev/arduino-1.6.4/hardware/tools/avr/etc/avrdude.conf -v -pattiny85 -cstk500v1 -P/dev/ttyACM0 -b19200 -U flash:w:pinecone2.hex:i

USBTINY=$(shell lsusb -d 1781:0c9f | cut -f 2,4 -d ' ' | sed s/':'// | sed s/' '/':'/)

fuses_p3:
	$(AVRDUDE) -C/home/djwillia/dev/arduino-1.6.4/hardware/tools/avr/etc/avrdude.conf -v -v -v -v -pattiny85 -cusbtiny -Pusb:$(USBTINY) -e -Uefuse:w:0xff:m -Uhfuse:w:0xdf:m -Ulfuse:w:0xe2:m

install_p3: pinecone2.hex
	$(AVRDUDE) -C/home/djwillia/dev/arduino-1.6.4/hardware/tools/avr/etc/avrdude.conf -v -pattiny85 -cusbtiny -Pusb:$(USBTINY) -U flash:w:pinecone2.hex:i

clean:
	rm -f *.elf *.hex *.o

