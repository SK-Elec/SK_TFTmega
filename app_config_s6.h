/*
 * app_config.h
 *
 * Created: 29/01/2016 10:22:42 PM
 *  Author: Steve Wurst
 */

#ifndef APP_CONFIG_H_
#define APP_CONFIG_H_

#define F_CPU	8000000UL		// 8Mhz
#define BAUD	38400
#define MYUBRR	8000000/16/BAUD-1

#define Setb(port,bitnum)		port |= _BV(bitnum)
#define Clrb(port,bitnum)		port &= ~(_BV(bitnum))
#define Chkb(port,bitnum)		((port) & (1 << bitnum))
#define Bit(bitnum)				(1 << bitnum)

#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <avr/eeprom.h>
#include <avr/wdt.h>

#include "lcd_s6.h"
#include "functions.h"
#include "font_sans.h"
#include "pix.h"

#endif /* APP_CONFIG_H_ */