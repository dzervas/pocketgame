#include <avr/io.h>
#include <util/delay.h>

//#define SUART_ENABLE
#define PS2_ENABLE

#ifdef SUART_ENABLE
#include "suart.h"
#endif // SUART_ENABLE

#ifdef PS2_ENABLE
#include "ps2.h"
#endif // PS2_ENABLE

#define PIN_SHIFT_CLOCK	0
#define PIN_SHIFT_LATCH	3
#define PIN_SHIFT_DATA	4
//#define PIN_LED		5

#define SHIFT_SIZE	8	// Number of buttons on shift register, not the actual size

unsigned short int i = 0;

unsigned char bstate = 0;

void bcheck();

int main () {
	// Initiate pins
	DDRB = 0xFF;
	DDRB &= ~(1 << PIN_SHIFT_DATA); // Shift register data input
	PORTB |= (1 << PIN_SHIFT_CLOCK);

	#ifdef SUART_ENABLE
	// Initiate Software UART
	// Run at 16MHz
	// clock_prescale_set(clock_div_1);
	initserial(38400);
	#endif // SUART_ENABLE

	#ifdef PS2_ENABLE
	ps2init();
	#endif // PS2_ENABLE

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
			#ifdef SUART_ENABLE
			putch((char) (((int) '0') + i));
			#endif // SUART_ENABLE
		}
			#ifdef PS2_ENABLE
			ps2write(0x29);
			_delay_ms(50);
			ps2write(0xF0);
			_delay_ms(10);
			ps2write(0x29);
			_delay_ms(200);
			#endif // PS2_ENABLE
	}
}
