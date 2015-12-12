#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdint.h>
#include <util/delay.h>

#define BAUD         57600
#define BAUDRATE     ((F_CPU/(BAUD*16UL))-1)

uint8_t prevx = 0, prevy = 0, prevb = 0;
uint32_t count = 0;
volatile uint8_t x = 0, y = 0, b = 0;
volatile uint16_t time = 0;

void uputc(unsigned char c) {
  	while(!(UCSR0A & (1<<UDRE0)));
	UDR0 = c;
}

int main () {
	/* Hack: the UART is slow enough that debouncing is not needed */
	DDRD = 0;
	PORTD |= (_BV(4) | _BV(5) | _BV(6) | _BV(7));

	UBRR0H = (BAUDRATE >> 8);
	UBRR0L = BAUDRATE;
	UCSR0A = 0;
	UCSR0B |= (_BV(TXEN0) | _BV(RXEN0));
	UCSR0C |= (_BV(UCSZ00) | _BV(UCSZ01));

	/*** Setup pin change interrupt for PORTD ***/
	PCICR |= _BV(PCIE2); /* Enable PORTD interrupt */
	PCMSK2 |= (_BV(PCINT20) | _BV(PCINT21) | _BV(PCINT22) | _BV(PCINT23)); /* Enable pins */

	/*** Setup ADC on A1,2 with interrupt ***/
	ADMUX = _BV(REFS0) | _BV(ADLAR) | _BV(MUX0); /* Vcc reference and left adjust */
	/* Enable ADC, enable interrupt and 128 prescale*/
	ADCSRA = _BV(ADEN) | _BV(ADIE) | _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0);

	/*** Setup Timer0 interrupt ***/
	TCNT0 = 0x00; /* Clear timer */
	TIMSK0 |= _BV(TOIE0); /* Interrupt on counter0 overflow */
	TCCR0B |= _BV(CS00); /* No prescaling */

	sei();

	ADCSRA |= _BV(ADSC);
	
	while(1) {
		/* Don't send more than the baud can manage, you build a queue!
		   Also do not send without a reason... */
		if (count < (BAUD - 128) && (prevb != b || prevx != x || prevy != y)) {
			uputc(0xFD); /* Raw mode */
			uputc(0x06); /* Data length */
			uputc(128 - y); /* Y First */
			uputc(128 - x); /* X First */
			uputc(0x00); /* Y Second */
			uputc(0x00); /* X Second */
			uputc(b); /* First button byte */
			uputc(0x00); /* Second button byte */
			/* Dirty hack. Without this it seems to be building a queue even
			   with the timing workaround... */
			_delay_ms(10);

			count += 64;
			prevb = b;
			prevx = x;
			prevy = y;
		}
		if (time >= (F_CPU/256)) {
			time = 0;
			count = 0;
		}
	}
}

ISR(PCINT2_vect) {
	b = ((~PIND) & 0xF0) >> 4;
}

ISR(ADC_vect) {
	if (ADMUX & _BV(MUX0)) {
		y = ADCH;
		ADMUX = _BV(REFS0) | _BV(ADLAR) | _BV(MUX1);
	} else if (ADMUX & _BV(MUX1)) {
		x = ADCH;
		ADMUX = _BV(REFS0) | _BV(ADLAR) | _BV(MUX0);
	}

	ADCSRA |= _BV(ADSC);
}

ISR(TIMER0_OVF_vect) {
	++time;
}
