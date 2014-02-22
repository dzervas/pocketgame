# Settings
CC = avr-gcc
OBJCOPY = avr-objcopy
DUDE = avrdude
SIZE = avr-size
TARGET = attiny85
CLOCK = 16500000

# Recommended build options
CFLAGS = -Wall -std=c99 -pedantic -Ofast -mmcu=$(TARGET) -DF_CPU=$(CLOCK)
OBJFLAGS = -j .text -j .data -O ihex
DUDEFLAGS = -p $(TARGET) -c avrisp -b 19200 -P /dev/ttyACM0

# Object files for the firmware
OBJECTS = main.o

# Handle USB/UART includes
CFLAGS += -Isuart
OBJECTS += suart/suart.o

# By default, build the firmware, but do not flash it
all: main.hex

# With this, you can flash the firmware
flash: main.hex
	$(DUDE) $(DUDEFLAGS) -U flash:w:$<

# Housekeeping if you want it
clean:
	$(RM) *.o *.hex *.elf */*.o

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
