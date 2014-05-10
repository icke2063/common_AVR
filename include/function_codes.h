/*
 * function_codes.h
 *
 *  Created on: 10.05.2014
 *      Author: icke
 */

#ifndef FUNCTION_CODES_H_
#define FUNCTION_CODES_H_

/**
 * some defines for pin functions
 */
#define	PIN_DISABLED			0x00		/* pin unused */
#define PIN_INPUT				0x01		/* pin used for input */
#define PIN_SWITCH				0x02		/* pin used for switching */
#define PIN_TOGGLE				0x03		/* toggle current pin status */
#define PIN_PULSE				0x04		/* pin used for "short" impulse */
#define PIN_S0					0x05		/* pin used for S0 bus (not ISDN), but only usable on interrupt pins */
#define PIN_UART				0x06		/* pins used for uart in/output */
#define PIN_OW_POWER_PARASITE	0x07		/* one-wire pin */
#define PIN_OW_POWER_EXTERN		0x08		/* one-wire pin */
#define PIN_ADC					0x09		/* ADC pin */


#endif /* FUNCTION_CODES_H_ */
