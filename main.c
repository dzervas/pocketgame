#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdint.h>
#include <util/delay.h>

#define BAUD         57600
#define BAUDRATE     ((F_CPU/(BAUD*16UL))-1)
//#define ENABLE_ADC

uint8_t prevx = 128, prevy = 128;
uint8_t prevb = 0;
uint16_t b = 0;
uint32_t count = 0;
volatile uint8_t x = 128, y = 128;
volatile uint16_t time = 0;

void uputc(unsigned char c) {
  	while(!(UCSRA & (1<<UDRE)));
	UDR = c;
}

int main () {
	/* Hack: the UART is slow enough that debouncing is not needed */
	DDRB = 0;
	PORTB |= 0xFF;

	UBRRH = (BAUDRATE >> 8);
	UBRRL = BAUDRATE;
	UCSRA = 0;
	UCSRB |= _BV(TXEN);
	UCSRC |= (_BV(UCSZ0) | _BV(UCSZ1));

#ifdef ENABLE_ADC
	/*** Setup ADC on A1,2 with interrupt ***/
	ADMUX = _BV(REFS0) | _BV(ADLAR) | _BV(MUX0); /* Vcc reference and left adjust */
	/* Enable ADC, enable interrupt and 128 prescale*/
	ADCSRA = _BV(ADEN) | _BV(ADIE) | _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0);
#endif /* ENABLE_ADC */

	/*** Setup Timer0 interrupt ***/
	TCNT0 = 0x00; /* Clear timer */
	TIMSK |= _BV(TOIE0); /* Interrupt on counter0 overflow */
	TCCR0B |= _BV(CS00); /* No prescaling */

	sei();

#ifdef ENABLE_ADC
	ADCSRA |= _BV(ADSC);
#endif /* ENABLE_ADC */
	
	while(1) {
		b = (b & 0xFF00) | PINB;
		b = (b & 0xFF) | (PIND & (0xFF << 2));

		/* Don't send more than the baud can manage, you build a queue!
		   Also do not send without a reason... */
		if (count < (BAUD - 128) && (prevb != b || prevx != x || prevy != y)) {
			uputc(0xFD); /* Raw mode */
			uputc(0x06); /* Data length */
			uputc(128 - y); /* Y First */
			uputc(128 - x); /* X First */
			uputc(0x00); /* Y Second */
			uputc(0x00); /* X Second */
			uputc(b & 0xFF); /* First button byte */
			uputc((b & 0xFF00) >> 8); /* Second button byte */
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

ISR(TIMER0_OVF_vect) {
	++time;
}

#ifdef ENABLE_ADC
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
#endif /* ENABLE_ADC */
