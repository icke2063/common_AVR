/*
 * IO_handling.c
 *
 *  Created on: 31.01.2013
 *      Author: icke
 */

#include "IO_handling.h"
#include <util/delay.h>


unsigned char readIOpins(struct IO_octet *curPins){
	unsigned char func = 0;								/* pin function code */
	unsigned char tmp_PIN = 0x00;						/* buffer for pin status -> port status */

	unsigned char *cur_PIN=0;


	for (int pin = 0; pin < 8; pin++){										/* loop over all virtual IO Pins */

		cur_PIN = curPins->pins[pin].PPIN;

		switch (curPins->pins[pin].function_code) {
			case PIN_DISABLED:
				cbi(tmp_PIN,(pin%8));										/* clear bit in buffer */
				break;
			case PIN_INPUT: 												/* input pin -> read pin status to buffer */

				if ( bis(*cur_PIN,curPins->pins[pin].pin) ) {			/* get status of current pin */
					//set "ON"
					sbi(tmp_PIN,pin%8);										/* set bit */
				} else {
					//set "OFF"
					cbi(tmp_PIN,pin%8);										/* clear bit */
				}
				break;
			case PIN_SWITCH: 												/* switch pin -> read current pin status to buffer */

				if ( bis(*cur_PIN,curPins->pins[pin].pin) ) {			/* get status of current pin */
					//set "ON"
					cbi(tmp_PIN,pin%8);										/* set bit */
				} else {
					//set "OFF"
					sbi(tmp_PIN,pin%8);										/* clear bit */
				}
				break;
			case PIN_PULSE:
				/* switch pin -> read current pin status to buffer
				 * - same behaviour as switch pin
				 * - even the "short" on pulse can detected by this function
				 * - normally the off state would be detected by this function
				 */

				if ( bis(*cur_PIN,curPins->pins[pin].pin) ) {			/* get status of current pin */
					//set "ON"
					cbi(tmp_PIN,pin%8);										/* set bit */
				} else {
					//set "OFF"
					sbi(tmp_PIN,pin%8);										/* clear bit */
				}
				break;
			case PIN_S0:
				/*
				 * S0 functionality
				 * - check ddr/port/pin for S0 pin (only interrupt pins possible for this function)
				 * - write current counter value to virtual pin data area of txbuffer
				 */
/*				if((cur_PORT == P_S0_0_PORT) && (cur_DDR == P_S0_0_DDR) && (io_pins[pin].pin == S0_0_PIN)){
					txbuffer[VIRTUAL_DATA_START+(pin*sizeof(uint32_t))] = get_S0_data(0);
				}
				if((cur_PORT == P_S0_1_PORT) && (cur_DDR == P_S0_1_DDR) && (io_pins[pin].pin == S0_1_PIN)){
					txbuffer[VIRTUAL_DATA_START+(pin*sizeof(uint32_t))] = get_S0_data(1);
				}*/
				break;

			default:														/* default handling */
				sbi(tmp_PIN,(pin%8));										/* set bit */
				break;
		}

	}
	return tmp_PIN;
}

void handleIOpins(struct IO_octet *curPins, unsigned char instructions) {

	unsigned char *cur_DDR=0;												/* pointer to DDR register */
	unsigned char *cur_PORT=0;												/* pointer to I/O Port */

/*	unsigned char S0_0 = 0;
	unsigned char S0_1 = 0;*/


	for (int pin = 0; pin < 8; pin++) {										/* loop over all virtual IO Pins */

		/*
		 * first get all needed address information for current pin/port
		 * - the "action" data is written by i2c into the rxbuffer -> need correct rxbuffer address
		 * - the function of the current pin is readout from the eeprom -> need correct eeprom address
		 */


		cur_PORT = curPins->pins[pin].PPORT;							/* get pointer to current port */
		cur_DDR	 = curPins->pins[pin].PDDR;								/* get pointer to current datadirectionregister */

		switch (curPins->pins[pin].function_code) {														/* handle current pin by function code */
		case PIN_DISABLED:													/* pin is marked as disabled */
			*cur_DDR	&= ~(1 << curPins->pins[pin].pin); 				//disable PIN
			*cur_PORT	&= ~(1 << curPins->pins[pin].pin); 				//disable PULLUP
			break;
		case PIN_INPUT: 													/* pin marked as input -> set as input nothing else to to (value is readout @readIOpins() */
			*cur_DDR &= ~(1 << curPins->pins[pin].pin); 				//set DDR "0"
			*cur_PORT |= (1 << curPins->pins[pin].pin); 				//enable pullup PORT "1"
			break;
		case PIN_SWITCH: 													/* pin marked as output -> set as output and set the value of the specific bit of rxbuffer*/
			*cur_DDR |= (1 << curPins->pins[pin].pin); 					/* set DDR "1" -> output */
			if ( bis(instructions,(pin%8)) ) {
				*cur_PORT &= ~(1 << curPins->pins[pin].pin);			//set "ON"
			} else {
				*cur_PORT |= (1 << curPins->pins[pin].pin);				//set "OFF"
			}
			break;
		case PIN_PULSE: 													/* pin marked as output -> set as output and pulse the specific pin */
			*cur_DDR |= (1 << curPins->pins[pin].pin); 					/* set DDR "1" -> output */
			if( bis(instructions,(pin%8)) ){						//only handle active pins
				*cur_PORT &= ~(1 << curPins->pins[pin].pin); 			//set ON
				_delay_ms(300); 											/* wait 300ms @todo set eeprom address for dynamic usage */
				cbi(instructions,pin%8); 						/* set bit to 0 */
			}
			*cur_PORT |= (1 << curPins->pins[pin].pin); 				//set OFF

			break;
		case PIN_S0:
			/*
			 * S0 functionality
			 *
			 * @todo test this function
			 */

			/*//INT0
			if((cur_PORT == P_S0_0_PORT) && (cur_DDR == P_S0_0_DDR) && (io_pins[pin].pin == S0_0_PIN)){
				//enable S0
				*cur_DDR &= ~(1 << io_pins[pin].pin); //set DDR "0"
				*cur_PORT |= (1 << io_pins[pin].pin); //enable pullup PORT "1"
				S0_Start(0);
				S0_0 = 1;
				if(get_S0_data(0)%S0_interval == 0){
					save_eeprom(0);
				}
				break;
			}else{
				//INT1
				if((cur_PORT == P_S0_1_PORT) && (cur_DDR == P_S0_1_DDR) && (io_pins[pin].pin == S0_1_PIN)){

					//enable S0
					*cur_DDR &= ~(1 << io_pins[pin].pin); //set DDR "0"
					*cur_PORT |= (1 << io_pins[pin].pin); //enable pullup PORT "1"
					S0_Start(1);
					S0_1 = 1;
					if(get_S0_data(1)%S0_interval == 0){
						save_eeprom(1);
					}
					break;
				}else{
					eeprom_busy_wait ();
					eeprom_write_byte(eeprom_address,PIN_DISABLED);	//write function code to eeprom
				}
			}*/
			break;
		default:
			*cur_DDR &= ~(1 << curPins->pins[pin].pin); //disable PIN
			*cur_PORT &= ~(1 << curPins->pins[pin].pin); //disable PULLUP
			break;
			}
	}

/*	//S0 deaktivieren
	if(!S0_0){
		S0_Stop(0);
	}

	if(!S0_1){
		S0_Stop(1);
	}*/

}

