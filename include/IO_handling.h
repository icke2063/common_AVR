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

/**
 * some defines for pin functions
 */
#define	PIN_DISABLED		0x00		/* pin unused */
#define PIN_INPUT			0x01		/* pin used for input */
#define PIN_SWITCH			0x02		/* pin used for switching */
#define PIN_PULSE			0x03		/* pin used for "short" impulse */
#define PIN_S0				0x04		/* pin used for S0 bus (not ISDN), but only usable on interrupt pins */
#define PIN_UART			0x05		/* pins used for uart in/output */
#define PIN_OW				0x06		/* one-wire pin */
#define PIN_ADC				0x07		/* ADC pin */

/* structure with all needed address and function data */
struct IO_pin{
unsigned char *PPORT;
unsigned char *PPIN;
unsigned char *PDDR;
unsigned char pin;
unsigned char function_code;
};

struct IO_octet{
	struct IO_pin pins[8];
};

#ifndef MAX_IO_PINS
	#define MAX_IO_PINS	8
#endif

#ifndef MAX_VIRT_IO_PORT
	#define	MAX_VIRT_IO_PORT	(MAX_IO_PINS/8)+1	/* calc count of virtual port memory */
#endif

extern struct IO_octet io_pins[MAX_VIRT_IO_PORT];

/**
 * @brief initializing function for virtual IO pins
 */
extern void initIOpins(void);

/**
 * @brief get input data of all input pins
 * @return byte of current situationon virtual input pins
 */
unsigned char readIOpins(struct IO_octet *curPins);

/**
 * @brief set output pins by instruction byte
 *
 * @param curPins pointer to current pin structure (8 pins)
 * @param instructions instruction byte how to set the output pins
 */
void handleIOpins(struct IO_octet *curPins,unsigned char instructions);
#endif /* IO_HANDLING_H_ */
