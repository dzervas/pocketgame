#include <avr/io.h>
#include "suart.h"

#define PIN_LED		0
#define PIN_BUTTON	3

uint8_t state = 0;

int main () {
	// Initiate pins
	DDRB |= (1 << PIN_LED);

	// Initiate Software UART for RN-42 communication
	initserial(115200);

	while(1) {
		// TODO: Needs debouncing
		// TODO: Add more button handling
		if (state && (PINB & (1 << PIN_BUTTON))) {
			putch(0xFE);
			putch(0x2);
			putch(0x0);
			putch('a');
			PORTB |= (1 << PIN_LED);
			state = 0;
		} else if (!state && !(PINB & (1 << PIN_BUTTON))) {
			putch(0xFE);
			putch(0x0);
			PORTB &= ~(1 << PIN_LED);
			state = 1;
		}
	}
}
