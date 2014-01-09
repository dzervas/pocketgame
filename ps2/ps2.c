#include "ps2.h"

void clockpulse() {
	_delay_us(CLK_HALF);
	// Clock low
	CLOCK_PORT &= ~(1 << CLOCK_PIN);
	/*
	CLOCK_PORT &= ~_BV(CLOCK_PIN); // zero output value
	CLOCK_DDR |= _BV(CLOCK_PIN); // set as output
	*/
	_delay_us(CLK_FULL);
	// Clock high
	CLOCK_PORT |= (1 << CLOCK_PIN);
	/*
	CLOCK_DDR &= ~_BV(CLOCK_PIN); // set as input
	CLOCK_PORT |= _BV(CLOCK_PIN); // set pullup
	*/
	_delay_us(CLK_HALF);
}

void datahigh() {
	DATA_PORT |= (1 << DATA_PIN);
	/*
	DATA_DDR &= ~_BV(DATA_PIN); // set as input
	DATA_PORT |= _BV(DATA_PIN); // set pullup
	*/
}

void datalow() {
	DATA_PORT &= ~(1 << DATA_PIN);
	/*
	DATA_PORT &= ~_BV(DATA_PIN); // zero output value
	DATA_DDR |= _BV(DATA_PIN); // set as output
	*/
}

void ps2init() {
	CLOCK_DDR |= _BV(CLOCK_PIN); // set as input
	CLOCK_PORT |= _BV(CLOCK_PIN); // set pullup
	DATA_DDR |= _BV(DATA_PIN); // set as input
	datahigh();
}

int ps2write(unsigned char data) {
	unsigned char i;
	unsigned char parity = 0;

	if(!(CLOCK_INPUT & (1 << CLOCK_PIN)) || !(DATA_INPUT & (1 << DATA_PIN)))
		return -1; // clock or data low

	// Start bit
	datalow();
	clockpulse();

	// Data byte
	for(i=0; i<8; i++) {
		if(data & 1) {
			datahigh();
			parity++;
		} else
			datalow();

		clockpulse();
		data >>= 1;
	}

	// Parity bit
	if(parity & 1)
		datahigh();
	else
		datalow();
	clockpulse();

	// Stop bit
	datahigh();
	clockpulse();

	_delay_us(50);

	return 0;
}
