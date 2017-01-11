/*
 * IO_handling.h
 *
 *  Created on: 31.01.2013
 *      Author: icke
 *
 *
 *
 *      simple library for common IO handling on AVR
 *      - virtual IO ports (individual mapping for each physical IO pin)
 *      - set special "functions" to pins for extended usage (e.g. S0 bus)
 */

#ifndef IO_HANDLING_H_
#define IO_HANDLING_H_
#include <stdint.h>
#include <1-wire_config.h>
#include "function_codes.h"

//#define IO_DEBUG usart_write
#define IO_DEBUG(...)

//#define READ_IO_DEBUG IO_DEBUG
#define READ_IO_DEBUG(...)

#define INIT_IO_DEBUG IO_DEBUG
//#define INIT_IO_DEBUG(...)

#define HANDLE_IO_DEBUG IO_DEBUG
//#define HANDLE_IO_DEBUG(...)

//###################### Macros (jtronics)
#define uniq(LOW,HEIGHT)	((HEIGHT << 8)|LOW)			// 2x 8Bit 	--> 16Bit
#define LOW_BYTE(x)        	(x & 0xff)					// 16Bit 	--> 8Bit
#define HIGH_BYTE(x)       	((x >> 8) & 0xff)			// 16Bit 	--> 8Bit


#define sbi(ADDRESS,BIT) 	((ADDRESS) |= (1<<(BIT)))	// set Bit
#define cbi(ADDRESS,BIT) 	((ADDRESS) &= ~(1<<(BIT)))	// clear Bit
#define	toggle(ADDRESS,BIT)	((ADDRESS) ^= (1<<BIT))		// Bit umschalten

#define	bis(ADDRESS,BIT)	(ADDRESS & (1<<BIT))		// bit is set?
#define	bic(ADDRESS,BIT)	(!(ADDRESS & (1<<BIT)))		// bit is clear?

#define DEFAULT_PULSE_TIME	300
extern uint16_t pulse_time;

/* structure with all needed address and function data */
struct IO_pin{
unsigned char *PPORT;			/* physical port pointer (output) */
unsigned char *PPIN;			/* physical pin pointer (input) */
unsigned char *PDDR;			/* data direction pointer */
unsigned char pin;				/* pin number of physical port */
uint8_t function_code;	/* pin function code (see defines above) */
};

/**
 * pin count for one virtual port
 * - should have same size as the request/return value of the handle/read functions
 */
#define VIRTUAL_PORT_PINCOUNT	8

struct virtual_IO_port{
	struct IO_pin pins[VIRTUAL_PORT_PINCOUNT];
};

/* calc count of virtual port memory */
#define GET_VIRT_PORT_COUNT(count_io_pins) (count_io_pins/8)+((count_io_pins%8)>0?1:0)

/**
 * @brief initializing function for virtual IO ports
 */
void initIOport(struct virtual_IO_port *virtualPort);

/**
 * @brief get input data of all virtual input pins
 * @return byte of current virtual input port
 */
unsigned char readvirtIOport(struct virtual_IO_port *virtualPort);

/**
 * @brief set output pins by instruction byte
 *
 * @param curPins pointer to current pin structure (8 pins)
 * @param instructions instruction byte how to set the output pins
 */
void handleIOport(struct virtual_IO_port *virtualPort,unsigned char instructions, const unsigned char mask);
#ifdef USE_OW
	void set1WirePin(struct IO_pin *vpin, uint8_t power_mode);
	void read1WirePin(struct IO_pin *vpin, uint8_t power_mode);
#endif
#endif /* IO_HANDLING_H_ */
