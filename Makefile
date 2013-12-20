# WinAVR cross-compiler toolchain is used here
CC = avr-gcc
OBJCOPY = avr-objcopy
DUDE = avrdude

# If you are not using ATtiny2313 and the USBtiny programmer,
# update the lines below to match your configuration
CFLAGS = -Wall -std=c99 -Os -mmcu=attiny85 -DF_CPU=8000000
OBJFLAGS = -j .text -j .data -O ihex
DUDEFLAGS = -p attiny85 -c avrisp -b 19200 -P /dev/ttyACM0

# Object files for the firmware (usbdrv/oddebug.o not strictly needed I think)
OBJECTS = main.o

# By default, build the firmware and command-line client, but do not flash
all: main.hex

# With this, you can flash the firmware by just typing "make flash" on command-line
flash: main.hex
	$(DUDE) $(DUDEFLAGS) -U flash:w:$<

# Housekeeping if you want it
clean:
	$(RM) *.o *.hex *.elf

# From .elf file to .hex
%.hex: %.elf
	$(OBJCOPY) $(OBJFLAGS) $< $@

# Main.elf requires additional objects to the firmware, not just main.o
main.elf: $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $@

# From C source to .o object file
%.o: %.c	
	$(CC) $(CFLAGS) -c $< -o $@

# From assembler source to .o object file
%.o: %.S
	$(CC) $(CFLAGS) -x assembler-with-cpp -c $< -o $@
