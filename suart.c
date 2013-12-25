#include "suart.h"

static uint16_t bit_time, start_bit_time;
static volatile uint8_t srx_done, stx_count;
static volatile uint8_t srx_data, srx_mask, srx_tmp, stx_data;

char getch();
char getche();
void initserial(unsigned long int baud);
char kbhit();
void putch(char c);
void putstr(const char *s);

char getch() {
	while (!srx_done);
	srx_done = 0;
	return srx_data;
}

char getche() {
	char c = getch();
	putch(c);
	return c;
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
	PORTB |= 1 << UART_TX;
	DDRB |= 1 << UART_TX;
	PORTB |= (1 << UART_RX);
	sei();
}

char kbhit() {
	return srx_done;
}

void putch(char c) {
	while (stx_count);
	stx_data = ~c;
	stx_count = 10;
}

void putstr(const char *s) {
	// Usage: putstr(PSTR("string"));
	while (pgm_read_byte(s))
		putch(pgm_read_byte(s++));
}

ISR(PCINT0_vect) {
	if (PINB & (1 << UART_RX))
		return;

	OCR1B = TCNT1 + (uint16_t) (start_bit_time);
	srx_tmp = 0;
	srx_mask = 1;
	TIFR = 1 << OCF1B;
	if ((!(PINB & (1 << UART_RX)))) {
		TIMSK = (1 << OCIE1A) ^ (1 << OCIE1B);
		PCMSK &= ~(1 << PCINT0);
	}

}

ISR(TIMER1_COMPB_vect) {
	uint8_t in = PINB;
	if (srx_mask) {
		if (in & (1 << UART_RX))
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
