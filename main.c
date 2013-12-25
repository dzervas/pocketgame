#include <avr/io.h>
#include <util/delay.h>

//#define SUART
#ifdef SUART
#include "suart.h"
#endif // SUART

#define PIN_SHIFT_CLOCK	2
#define PIN_SHIFT_LATCH	3
#define PIN_SHIFT_DATA	4
#define PIN_LED		5

#define SHIFT_SIZE	8	// Number of buttons on shift register, not the actual size

unsigned short int i = 0;

unsigned char bstate = 0;

void bcheck();

int main () {
	// Initiate pins
	DDRB = 0xFF;
	DDRB &= ~(1 << PIN_SHIFT_DATA);
	PORTB |= (1 << PIN_SHIFT_CLOCK);
	PORTB |= (1 << PIN_LED);
	_delay_ms(500);
	PORTB &= ~(1 << PIN_LED);

	#ifdef SUART
	// Initiate Software UART
	// Run at 16MHz
	clock_prescale_set(clock_div_1);
	initserial(38400);
	#endif // SUART

	while(1) {
		bcheck();
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

		if (bstate & (1 << PIN_SHIFT_DATA)) {
			#ifdef SUART
			putch((char) (((int) '0') + i));
			#endif // SUART

			if (i == 7)
				PORTB |= (1 << PIN_LED);
		} else if (i == 7)
			PORTB &= ~(1 << PIN_LED);

	}
}
