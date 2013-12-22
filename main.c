#include <avr/io.h>
#include <util/delay.h>
#include "main.h"
#include "suart.h"

#define PIN_SHIFT_CLOCK	0
#define PIN_SHIFT_LATCH	1
#define PIN_SHIFT_DATA	2
#define PIN_LED		3

#define SHIFT_SIZE	8	// Number of buttons on shift register, not the actual size

unsigned short int i = 0;

unsigned char bstate = 0;

void bcheck();

int main (void) {
	// Run at 16MHz
	clock_prescale_set(clock_div_1);

	// Initiate pins
	DDRB = 0xFF;
	DDRB &= ~(1 << PIN_SHIFT_DATA);
	PORTB |= (1 << PIN_SHIFT_CLOCK);
	PORTB |= (1 << PIN_LED);
	_delay_ms(500);
	PORTB &= ~(1 << PIN_LED);

	// Initiate UART
	suart_init();
	sei();
	uputs("Hello World!\n\r");

	while(1) {
		//bcheck();
		while(kbhit())
			uputchar( ugetchar());
		_delay_ms(1000);
	}
}

void bcheck() {
	PORTB |= (1 << PIN_SHIFT_LATCH);
	_delay_us(1);
	PORTB &= ~(1 << PIN_SHIFT_LATCH);
	
	for (i = 0; i < SHIFT_SIZE; i++) {
		PORTB &= ~(1 << PIN_SHIFT_CLOCK);
		bstate = PINB;
		PORTB |= (1 << PIN_SHIFT_CLOCK);

		if (i == 7 && (bstate & (1 << PIN_SHIFT_DATA)))
			PORTB |= (1 << PIN_LED);
		else if (i == 7)
			PORTB &= ~(1 << PIN_LED);
	}
}
