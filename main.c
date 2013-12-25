#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/power.h>
#include <util/delay.h>

#define PIN_SHIFT_CLOCK	0
#define PIN_SHIFT_LATCH	1
#define PIN_SHIFT_DATA	2
#define PIN_LED		3

#define SHIFT_SIZE	8	// Number of buttons on shift register, not the actual size

unsigned short int i = 0;
static uint16_t bit_time, start_bit_time;
static volatile uint8_t srx_done, stx_count;
static volatile uint8_t srx_data, srx_mask, srx_tmp, stx_data;

unsigned char bstate = 0;

void bcheck();

void initserial(unsigned long baud);
char kbhit(void);
char getch(void);
char getche(void);
void putch(char c);
void putstr(const char *s);

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

	initserial(38400);

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

		if (i == 7 && (bstate & (1 << PIN_SHIFT_DATA)))
			PORTB |= (1 << PIN_LED);
		else if (i == 7)
			PORTB &= ~(1 << PIN_LED);
	}
}

char getch(void) {
	while (!srx_done);
	srx_done = 0;
	return srx_data;
}

char kbhit(void) {
	return srx_done;
}

void putch(char c) {
	while (stx_count);
	stx_data = ~c;
	stx_count = 10;
}

char getche(void) {
	char c = getch();
	putch(c);
	return c;
}

void putstr(const char *s) {
	// Usage: putstr(PSTR("string"));
	while (pgm_read_byte(s))
		putch(pgm_read_byte(s++));
}

void initserial(unsigned long baud) {
	bit_time = (F_CPU / 8) / baud;
	start_bit_time = bit_time + (bit_time >> 1);
	OCR1A = TCNT1 + 1;
	TCCR1 = (4 << CS10);
	TCCR1 |= (1 << COM1A1) | (1 << COM1A0);
	TIMSK = (1 << OCIE1A);
	PCMSK |= (1 << PCINT0);
	GIMSK |= (1 << PCIE);
	stx_count = 0;
	srx_done = 0;
	PORTB |= 1 << 1; // TX
	DDRB |= 1 << 1;
	PORTB |= (1 << 0); // RX
	sei();
}

ISR(PCINT0_vect) {
	if (PINB & (1 << 0))
		return;
	OCR1B = TCNT1 + (uint16_t) (start_bit_time);
	srx_tmp = 0;
	srx_mask = 1;
	TIFR = 1 << OCF1B;
	if ((!(PINB & (1 << 0)))) {
		TIMSK = (1 << OCIE1A) ^ (1 << OCIE1B);
		PCMSK &= ~(1 << PCINT0);
	}

}

ISR(TIMER1_COMPB_vect) {
	uint8_t in = PINB;
	if (srx_mask) {
		if (in & (1 << 0))
			srx_tmp |= srx_mask;
		srx_mask <<= 1;
		OCR1B = OCR1B + bit_time;
	} else {
		srx_done = 1;
		srx_data = srx_tmp;
		TIMSK = (1 << OCIE1A);
		PCMSK |= (1 << PCINT0);
	}
}

ISR(TIMER1_COMPA_vect) {
	uint8_t count;

	OCR1A = OCR1A + bit_time;
	count = stx_count;
	if (count) {
		stx_count = --count;

		if (count != 9) {
			if (!(stx_data & 1))
				TCCR1 |= (1 << COM1A0);
			else
				TCCR1 &= ~(1 << COM1A0);
			stx_data >>= 1;
		} else {
			TCCR1 &= ~(1 << COM1A0);
		}
	} else {
	}
}
