#include <stdint.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/power.h>

#define UART_RX		0
#define UART_TX		1

extern char getch();
extern char getche();
extern void initserial(unsigned long int baud);
extern char kbhit();
extern void putch(char c);
extern void putstr(const char *s);
