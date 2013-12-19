#include <avr/io.h>
#include <util/delay.h>
#include "USI_UART.h"

#define PIN_SHIFT_LATCH	4
#define PIN_SHIFT_CLOCK	5
#define PIN_SHIFT_DATA	2
#define PIN_LED		3

#define SHIFT_SIZE	8	// Number of buttons on shift register, not the actual size
//#define SHIFT_DEBOUNCE	30	// Amount of debouncing time (ms)

unsigned short int i = 0;
//volatile unsigned long millis, bprevious[SHIFT_SIZE] = {0};

unsigned char bstate = 0;

void bcheck();

int main (void) {
	// Initiate pins
	DDRB = 0xFF;
	DDRB &= ~(1 << PIN_SHIFT_DATA);
	PORTB |= (1 << PIN_SHIFT_CLOCK);
	PORTB |= (1 << PIN_LED);
	_delay_ms(500);
	PORTB &= ~(1 << PIN_LED);

	// Initiate USI UART
	USI_UART_Initialise_Transmitter();

	while(1) {
		while(USI_UART_Data_In_Receive_Buffer())
			USI_UART_Transmit_Byte(USI_UART_Receive_Byte());
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

		// XXX: Check if debouncing is needed
		//if ((millis - bprevious[i]) < SHIFT_DEBOUNCE)
		//	continue;

		if (i == 7 && (bstate & (1 << PIN_SHIFT_DATA)))
			PORTB |= (1 << PIN_LED);
		else if (i == 7)
			PORTB &= ~(1 << PIN_LED);

		//bprevious[i] = millis;

	}
}

/*
ISR(TIMERx_COMP_vect) {
	millis++;
}
*/
