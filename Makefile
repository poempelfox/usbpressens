# $Id: Makefile $
# Makefile for USB barometric pressure sensor with display.

CC	= avr-gcc
OBJDUMP	= avr-objdump
OBJCOPY	= avr-objcopy
AVRDUDE	= avrdude
INCDIR	= .
# There are a few additional defines that en- or disable certain features,
# mainly to save space in case you are running out of flash.
# You can add them here.
#  -DSERIALCONSOLE   Support for serial debug console.
#                    Note that this project is not functional without a serial
#                    console, so this is not optional in this project.
ADDDEFS	= -DSERIALCONSOLE

# target mcu (atmega 328p)
MCU	= atmega328p
# Since avrdude is generally crappy software (I liked uisp a lot better, too
# bad the project is dead :-/), it cannot use the MCU name everybody else
# uses, it has to invent its own name for it. So this defines the same
# MCU as above, but with the name avrdude understands.
AVRDMCU	= m328p

# Some more settings
# Clock Frequency of the AVR. Needed for various calculations.
CPUFREQ		= 8000000UL
# desired baudrate of serial console
BAUDRATE	= 19200UL

SRCS	= console.c main.c mpl3115a2.c timers.c twi.c
PROG	= usbpressens

# compiler flags
CFLAGS	= -g -Os -Wall -Wno-pointer-sign -std=c99 -mmcu=$(MCU) $(ADDDEFS)

# linker flags
LDFLAGS = -g -mmcu=$(MCU) -Wl,-Map,$(PROG).map -Wl,--gc-sections
# For serialconsole to properly print human readable temp/hum values,
# we'll need to add floatingpoint.
# This is about 3 KB of bloat.
ifneq (,$(findstring -DSERIALCONSOLE,$(ADDDEFS)))
  LDFLAGS += -Wl,-u,vfprintf -lprintf_flt -lm
endif

CFLAGS += -DCPUFREQ=$(CPUFREQ) -DF_CPU=$(CPUFREQ) -DBAUDRATE=$(BAUDRATE)

OBJS	= $(SRCS:.c=.o)

all: compile dump text eeprom
	@echo -n "Compiled size: " && ls -l $(PROG).bin

compile: $(OBJS)
	$(CC) $(LDFLAGS) -o $(PROG).elf $(OBJS)

dump: compile
	$(OBJDUMP) -h -S $(PROG).elf > $(PROG).lst

%o : %c 
	$(CC) $(CFLAGS) -I$(INCDIR) -c $< -o $@

# Create the flash contents
text: compile
	$(OBJCOPY) -j .text -j .data -O ihex $(PROG).elf $(PROG).hex
	$(OBJCOPY) -j .text -j .data -O srec $(PROG).elf $(PROG).srec
	$(OBJCOPY) -j .text -j .data -O binary $(PROG).elf $(PROG).bin

# Rules for building the .eeprom rom images
eeprom: compile
	$(OBJCOPY) -j .eeprom --change-section-lma .eeprom=0 -O ihex $(PROG).elf $(PROG)_eeprom.hex
	$(OBJCOPY) -j .eeprom --change-section-lma .eeprom=0 -O srec $(PROG).elf $(PROG)_eeprom.srec
	$(OBJCOPY) -j .eeprom --change-section-lma .eeprom=0 -O binary $(PROG).elf $(PROG)_eeprom.bin

clean:
	rm -f $(PROG) *~ *.elf *.rom *.bin *.eep *.o *.lst *.map *.srec *.hex

upload: uploadflash

uploadflash:
	$(AVRDUDE) -v -c arduino -b 57600 -p $(AVRDMCU) -P /dev/ttyUSB0 -U flash:w:$(PROG).hex

# We currently use no EEPROM, nothing to upload.
#uploadeeprom:
#	$(AVRDUDE) -c stk500v2 -p $(AVRDMCU) -P /dev/ttyUSB0 -U eeprom:w:$(PROG)_eeprom.srec:s

