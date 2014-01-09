#include <avr/io.h>
#include <util/delay.h>

#define CLOCK_DDR DDRB
#define CLOCK_INPUT PINB
#define CLOCK_PIN 2
#define CLOCK_PORT PORTB

#define DATA_DDR DDRB
#define DATA_INPUT PINB
#define DATA_PIN 1
#define DATA_PORT PORTB

#define CLK_FULL 40
#define CLK_HALF 20

extern int ps2write(unsigned char data);
extern void ps2init();
