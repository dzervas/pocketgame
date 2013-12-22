#ifndef _main_h_
#define _main_h_


#include <avr/pgmspace.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "mydefs.h"


#define	XTAL	11.0592e6
#define	BAUD	38400

#define F_CPU   XTAL
#include <util/delay.h>

#define	STXD		SBIT( PORTB, PB1 )	// = OC1A
#define	STXD_DDR	SBIT( DDRB,  PB1 )

#define	SRXD_PIN	SBIT( PINB,  PB0 )	// = ICP


#endif
