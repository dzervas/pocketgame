#include <avr/io.h>
#include <util/delay.h>

//#define SUART_ENABLE
#define USB_ENABLE

#ifdef SUART_ENABLE
#include "suart.h"
#endif // SUART_ENABLE

#ifdef USB_ENABLE
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <avr/eeprom.h>
#include <avr/pgmspace.h>
//#if (USB_PUBlIC==static)
//#include <usbdrv.c>
//#endif
#include "usbdrv.h"

#define NUM_LOCK 1
#define CAPS_LOCK 2
#define SCROLL_LOCK 4
#define abs(x) ((x) > 0 ? (x) : (-x))

#define STATE_WAIT 0
#define STATE_SEND_KEY 1
#define STATE_RELEASE_KEY 2
#endif // USB_ENABLE

#define PIN_SHIFT_CLOCK	0
#define PIN_SHIFT_LATCH	3
#define PIN_SHIFT_DATA	4
//#define PIN_LED		5

#define SHIFT_SIZE	8	// Number of buttons on shift register, not the actual size

unsigned short int i = 0;

unsigned char bstate = 0;

void bcheck();
#ifdef USB_ENABLE
static unsigned char bflag[SHIFT_SIZE] = {0};
volatile static uchar LED_state = 0xff; // received from PC
static uchar idleRate; // repeat rate for keyboards
uchar state = STATE_SEND_KEY;

void buildReport(uchar send_key);
void hadUsbReset();
void initusb();
usbMsgLen_t usbFunctionSetup(uchar data[8]);
usbMsgLen_t usbFunctionWrite(uint8_t * data, uchar len);

typedef struct {
	uint8_t modifier;
	uint8_t reserved;
	uint8_t keycode[6];
} keyboard_report_t;

static keyboard_report_t keyboard_report; // sent to PC
// From Frank Zhao's USB Business Card project
// http://www.frank-zhao.com/cache/usbbusinesscard_details.php
PROGMEM const char usbHidReportDescriptor[USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH] = {
	0x05, 0x01,					// USAGE_PAGE (Generic Desktop)
	0x09, 0x06,					// USAGE (Keyboard)
	0xa1, 0x01,					// COLLECTION (Application)
	0x75, 0x01,					//   REPORT_SIZE (1)
	0x95, 0x08,					//   REPORT_COUNT (8)
	0x05, 0x07,					//   USAGE_PAGE (Keyboard)(Key Codes)
	0x19, 0xe0,					//   USAGE_MINIMUM (Keyboard LeftControl)(224)
	0x29, 0xe7,					//   USAGE_MAXIMUM (Keyboard Right GUI)(231)
	0x15, 0x00,					//   LOGICAL_MINIMUM (0)
	0x25, 0x01,					//   LOGICAL_MAXIMUM (1)
	0x81, 0x02,					//   INPUT (Data,Var,Abs) ; Modifier byte
	0x95, 0x01,					//   REPORT_COUNT (1)
	0x75, 0x08,					//   REPORT_SIZE (8)
	0x81, 0x03,					//   INPUT (Cnst,Var,Abs) ; Reserved byte
	0x95, 0x05,					//   REPORT_COUNT (5)
	0x75, 0x01,					//   REPORT_SIZE (1)
	0x05, 0x08,					//   USAGE_PAGE (LEDs)
	0x19, 0x01,					//   USAGE_MINIMUM (Num Lock)
	0x29, 0x05,					//   USAGE_MAXIMUM (Kana)
	0x91, 0x02,					//   OUTPUT (Data,Var,Abs) ; LED report
	0x95, 0x01,					//   REPORT_COUNT (1)
	0x75, 0x03,					//   REPORT_SIZE (3)
	0x91, 0x03,					//   OUTPUT (Cnst,Var,Abs) ; LED report padding
	0x95, 0x06,					//   REPORT_COUNT (6)
	0x75, 0x08,					//   REPORT_SIZE (8)
	0x15, 0x00,					//   LOGICAL_MINIMUM (0)
	0x25, 0x65,					//   LOGICAL_MAXIMUM (101)
	0x05, 0x07,					//   USAGE_PAGE (Keyboard)(Key Codes)
	0x19, 0x00,					//   USAGE_MINIMUM (Reserved (no event indicated))(0)
	0x29, 0x65,					//   USAGE_MAXIMUM (Keyboard Application)(101)
	0x81, 0x00,					//   INPUT (Data,Ary,Abs)
	0xc0						   // END_COLLECTION
};
#endif // USB_ENABLE

int main () {
	// Initiate pins
	//DDRB = 0xFF;
	//DDRB &= ~(1 << PIN_SHIFT_DATA); // Shift register data input
	//PORTB |= (1 << PIN_SHIFT_CLOCK);
	DDRB = 1 << PB0;
	//PORTB = 1 << PB3;

	#ifdef SUART_ENABLE
	// Initiate Software UART
	// Run at 16MHz
	clock_prescale_set(clock_div_1);
	initserial(38400);
	#endif // SUART_ENABLE

	#ifdef USB_ENABLE
	initusb();
	#endif // USB_ENABLE

	while(1) {
		#ifdef USB_ENABLE
		wdt_reset();
		usbPoll();
		if(!(PINB & (1<<PB3))) // button pressed
			state = STATE_SEND_KEY;
		if(usbInterruptIsReady() && state != STATE_WAIT && LED_state != 0xff){
			switch(state) {
			case STATE_SEND_KEY:
				buildReport('x');
				state = STATE_RELEASE_KEY; // release next
				break;
			case STATE_RELEASE_KEY:
				buildReport(NULL);
				state = STATE_WAIT; // go back to waiting
				break;
			}
		}
		#endif // USB_ENABLE

		//bcheck();
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

			#ifdef USB_ENABLE
			if (bflag[i] == 0 && usbInterruptIsReady()) {
				bflag[i] = 1;
				buildReport((char) (((int) '0') + i));
				usbSetInterrupt((void *)&keyboard_report, sizeof(keyboard_report));
				buildReport('\0'); // clear keyboard report
				usbSetInterrupt((void *)&keyboard_report, sizeof(keyboard_report));
			}
			#endif // USB_ENABLE
/*
			if (i == 7)
				PORTB |= (1 << PIN_LED);
*/
		} else {
/*
			if (i == 7)
				PORTB &= ~(1 << PIN_LED);
*/

			#ifdef USB_ENABLE
			if (bflag[i] == 1) {
				bflag[i] = 0;
			}
			#endif // USB_ENABLE
		}

	}
}

#ifdef USB_ENABLE
void buildReport(uchar send_key) {
	keyboard_report.modifier = 0;
	
	if(send_key >= 'a' && send_key <= 'z')
		keyboard_report.keycode[0] = 4+(send_key-'a');
	else
		keyboard_report.keycode[0] = 0;
}

// Called by V-USB after device reset
void hadUsbReset() {
	int frameLength, targetLength = (unsigned)(1499 * (double)F_CPU / 10.5e6 + 0.5);
	int bestDeviation = 9999;
	uchar trialCal, bestCal, step, region;

	// do a binary search in regions 0-127 and 128-255 to get optimum OSCCAL
	for(region = 0; region <= 1; region++) {
		frameLength = 0;
		trialCal = (region == 0) ? 0 : 128;

		for(step = 64; step > 0; step >>= 1) { 
			if(frameLength < targetLength) // true for initial iteration
				trialCal += step; // frequency too low
			else
				trialCal -= step; // frequency too high

			OSCCAL = trialCal;
			frameLength = usbMeasureFrameLength();

			if(abs(frameLength-targetLength) < bestDeviation) {
				bestCal = trialCal; // new optimum found
				bestDeviation = abs(frameLength -targetLength);
			}
		}
	}

	OSCCAL = bestCal;
}

void initusb() {
	for(i=0; i<sizeof(keyboard_report); i++) // clear report initially
		((uchar *)&keyboard_report)[i] = 0;

	wdt_enable(WDTO_1S); // enable 1s watchdog timer

	usbInit();

	usbDeviceDisconnect(); // enforce re-enumeration
	for(i = 0; i<250; i++) { // wait 500 ms
		wdt_reset(); // keep the watchdog happy
		_delay_ms(2);
	}
	usbDeviceConnect();

	TCCR0B |= (1 << CS01); // timer 0 at clk/8 will generate randomness

	sei(); // Enable interrupts after re-enumeration
}

usbMsgLen_t usbFunctionSetup(uchar data[8]) {
	usbRequest_t *rq = (void *)data;

	if((rq->bmRequestType & USBRQ_TYPE_MASK) == USBRQ_TYPE_CLASS) {
		switch(rq->bRequest) {
		case USBRQ_HID_GET_REPORT: // send "no keys pressed" if asked here
			// wValue: ReportType (highbyte), ReportID (lowbyte)
			usbMsgPtr = (void *)&keyboard_report; // we only have this one
			keyboard_report.modifier = 0;
			keyboard_report.keycode[0] = 0;
			return sizeof(keyboard_report);
		case USBRQ_HID_SET_REPORT: // if wLength == 1, should be LED state
			return (rq->wLength.word == 1) ? USB_NO_MSG : 0;
		case USBRQ_HID_GET_IDLE: // send idle rate to PC as required by spec
			usbMsgPtr = &idleRate;
			return 1;
		case USBRQ_HID_SET_IDLE: // save idle rate as required by spec
			idleRate = rq->wValue.bytes[1];
			return 0;
		}
	}
	
	return 0; // by default don't return any data
}

usbMsgLen_t usbFunctionWrite(uint8_t * data, uchar len) {
	if (data[0] == LED_state)
		return 1;
	else
		LED_state = data[0];
	
	// LED state changed
	if(LED_state & CAPS_LOCK)
		PORTB |= 1 << PB0; // LED on
	else
		PORTB &= ~(1 << PB0); // LED off
	
	return 1; // Data read, not expecting more
}
#endif // USB_ENABLE
