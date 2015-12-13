# Settings
CC = avr-gcc
OBJCOPY = avr-objcopy
DUDE = avrdude
SIZE = avr-size
SIM = simavr
UART = miniterm2.py 

TARGET = attiny2313
CLOCK = 12000000
PORT = /dev/ttyACM0

#CFLAGS = -Wall -Wno-overflow -pedantic -std=c99 -Ofast -mmcu=$(TARGET) -DF_CPU=$(CLOCK)
CFLAGS = -O3 -Wall -pedantic -std=c99 -mmcu=$(TARGET) -DF_CPU=$(CLOCK) -O1
OBJFLAGS = -j .text -j .data -O ihex
#DUDEFLAGS = -p $(TARGET) -c arduino -P $(PORT) -b 57600
#DUDEFLAGS = -p $(TARGET) -c arduino -P $(PORT) -b 19200
DUDEFLAGS = -p $(TARGET) -c usbtiny -B 1
FUSES = -U lfuse:w:0xEF:m -U hfuse:w:0xDF:m
UARTFLAGS = $(PORT) 57600

# Object files for the firmware
OBJECTS = $(patsubst %.c,%.o,$(wildcard *.c) $(wildcard crypto/*.c))

# By default, build the firmware, but do not flash it
all: main.hex

# With this, you can flash the firmware
flash: main.hex
	$(DUDE) $(DUDEFLAGS) -U flash:w:$<

fuse:
	$(DUDE) $(DUDEFLAGS) $(FUSES)

fresh: clean flash

simulate: main.hex
	$(SIM) -vvv -g -t -mcu $(TARGET) -freq $(CLOCK) main.hex

# Housekeeping if you want it
clean:
	$(RM) *.o *.hex *.elf

uart:
	$(UART) $(UARTFLAGS)

# From .elf file to .hex
%.hex: %.elf
	$(OBJCOPY) $(OBJFLAGS) $< $@
	$(SIZE) --mcu=$(TARGET) $@

# Main.elf requires additional objects to the firmware, not just main.o
main.elf: $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $@

# From C source to .o object file
%.o: %.c	
	$(CC) $(CFLAGS) -c $< -o $@

# From assembler source to .o object file
%.o: %.S
	$(CC) $(CFLAGS) -x assembler-with-cpp -c $< -o $@
