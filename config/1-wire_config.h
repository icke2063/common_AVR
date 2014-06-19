/*
 * 1-wire_config.h
 *
 *  Created on: 05.05.2014
 *      Author: icke
 */

#ifndef ONE_WIRE_CONFIG_H_
#define ONE_WIRE_CONFIG_H_

#include <avr/io.h>

//#define OW_ONE_BUS	1

#ifdef OW_ONE_BUS
	#define OW_PIN  PD1
	#define OW_IN   PIND
	#define OW_OUT  PORTD
	#define OW_DDR  DDRD
#endif

#endif /* ONE_WIRE_CONFIG_H_ */
