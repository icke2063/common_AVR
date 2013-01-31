/*
 * IO_handling.h
 *
 *  Created on: 31.01.2013
 *      Author: icke
 */

#ifndef IO_HANDLING_H_
#define IO_HANDLING_H_


//###################### Macros (jtronics)
#define uniq(LOW,HEIGHT)	((HEIGHT << 8)|LOW)			// 2x 8Bit 	--> 16Bit
#define LOW_BYTE(x)        	(x & 0xff)					// 16Bit 	--> 8Bit
#define HIGH_BYTE(x)       	((x >> 8) & 0xff)			// 16Bit 	--> 8Bit


#define sbi(ADDRESS,BIT) 	((ADDRESS) |= (1<<(BIT)))	// set Bit
#define cbi(ADDRESS,BIT) 	((ADDRESS) &= ~(1<<(BIT)))	// clear Bit
#define	toggle(ADDRESS,BIT)	((ADDRESS) ^= (1<<BIT))		// Bit umschalten

#define	bis(ADDRESS,BIT)	(ADDRESS & (1<<BIT))		// bit is set?
#define	bic(ADDRESS,BIT)	(!(ADDRESS & (1<<BIT)))		// bit is clear?


#define	PIN_DISABLED		0x00
#define PIN_INPUT		0x01
#define PIN_SWITCH		0x02
#define PIN_PULSE		0x03
#define PIN_S0			0x04
#define PIN_UART		0x05
#define PIN_OW			0x06
#define PIN_ADC			0x07

/* structure with all needed address data */
struct IO_pin{
const unsigned char *PPORT;
const unsigned char *PPIN;
const unsigned char *PDDR;
const unsigned char pin;
};

struct IO_octet{
	unsigned char function_code[8];
	struct IO_pin address[8];
	unsigned char input_buffer;
	unsigned char output_buffer;
};


void readIOpins(struct IO_octet *curPins);

#endif /* IO_HANDLING_H_ */
