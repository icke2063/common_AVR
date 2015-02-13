/*
 * IO_handling.c
 *
 *  Created on: 31.01.2013
 *      Author: icke
 */

#include "IO_handling.h"
#include <util/delay.h>
#include <stddef.h>
#ifdef USE_OW
	#include <ds18x20.h>
#endif

static uint16_t pulse_time = DEFAULT_PULSE_TIME;

unsigned char readvirtIOport(struct virtual_IO_port *virtualPort) {
	unsigned char pin;
	unsigned char result = 0x00; /* buffer for pin status -> port status */

	READ_IO_DEBUG("\r\nreadIOport\r\n");

	for (pin = 0; pin < VIRTUAL_PORT_PINCOUNT; pin++) { /* loop over all virtual IO Pins */
		READ_IO_DEBUG("pPort:0x%x;",virtualPort->pins[pin].PPORT);
		READ_IO_DEBUG("pDDR:0x%x;",virtualPort->pins[pin].PDDR);
		READ_IO_DEBUG("pPin:0x%x;",virtualPort->pins[pin].PPIN);
		READ_IO_DEBUG("pin:%i\r\n",pin);
		READ_IO_DEBUG("func:0x%x\r\n",virtualPort->pins[pin].function_code);

		if(virtualPort->pins[pin].PPORT == 0
				|| virtualPort->pins[pin].PDDR == 0
				|| virtualPort->pins[pin].PPIN == 0)
		{
			READ_IO_DEBUG("conti\r\n");
			continue;
		}

		switch (virtualPort->pins[pin].function_code) {
		case PIN_S0:
			/*
			 * S0 functionality
			 * - check ddr/port/pin for S0 pin (only interrupt pins possible for this function)
			 * - write current counter value to virtual pin data area of txbuffer
			 */
			break;
		case PIN_INPUT: /* input pin -> read pin status to buffer */
			if (bis(*(virtualPort->pins[pin].PPIN),virtualPort->pins[pin].pin)) { /* get status of current pin */
				//set "ON"
				cbi(result, pin%8);
				/* clear bit */
			} else {
				//set "OFF"
				sbi(result, pin%8);
				/* set bit */
			}
			break;
		case PIN_SWITCH: /* switch pin -> read current pin status to buffer */
		case PIN_TOGGLE:
		case PIN_PULSE:
			if (bis(*(virtualPort->pins[pin].PPORT),virtualPort->pins[pin].pin)) { /* get status of current pin */
				//set "ON"
				cbi(result, pin%8);
				/* clear bit */
			} else {
				//set "OFF"
				sbi(result, pin%8);
				/* set bit */
			}
			break;
#ifdef USE_OW
		case PIN_OW_POWER_PARASITE:
			read1WirePin(&virtualPort->pins[pin], DS18X20_POWER_PARASITE);
			break;
		case PIN_OW_POWER_EXTERN:
			read1WirePin(&virtualPort->pins[pin], DS18X20_POWER_EXTERN);
			break;
#endif
		case PIN_DISABLED:
		default: /* default handling */
			cbi(result, (pin%8));
			/* clear bit in buffer */
			break;
		}

	} READ_IO_DEBUG("result0x%x\r\n",result);
	return result;
}

void handleIOport(struct virtual_IO_port *virtualPort,
		unsigned char instructions, const unsigned char mask) {

	unsigned char *cur_DDR = 0; /* pointer to DDR register */
	unsigned char *cur_PORT = 0; /* pointer to O Port */
	unsigned char *cur_PIN = 0; /* pointer to I Pin */
	unsigned char cur_pin = 0;	/* pin number */

	HANDLE_IO_DEBUG("\r\nhandleIOport:0x%x\r\n",instructions);

	for (int pin = 0; pin < VIRTUAL_PORT_PINCOUNT; pin++) { /* loop over all virtual IO Pins */

		if (!(mask & (1 << pin)))
		{
			continue;
		}

		if(virtualPort->pins[pin].PPORT == 0
			|| virtualPort->pins[pin].PDDR == 0
			|| virtualPort->pins[pin].PPIN == 0)
		{
			continue;
		}

		/*
		 * first get all needed address information for current pin/port
		 * - the "action" data is written by i2c into the rxbuffer -> need correct rxbuffer address
		 * - the function of the current pin is readout from the eeprom -> need correct eeprom address
		 */

		cur_PORT = virtualPort->pins[pin].PPORT; /* get pointer to current port */
		HANDLE_IO_DEBUG("pPort:0x%x;",cur_PORT);

		cur_DDR = virtualPort->pins[pin].PDDR; /* get pointer to current datadirectionregister */
		HANDLE_IO_DEBUG("pDDR:0x%x;",cur_DDR);

		cur_PIN = virtualPort->pins[pin].PPIN; /* get pointer to current pin */
		HANDLE_IO_DEBUG("pPIN:0x%x\r\n",cur_PIN);

		cur_pin = virtualPort->pins[pin].pin;
		HANDLE_IO_DEBUG("pin:0x%x\r\n",cur_pin);

		HANDLE_IO_DEBUG("func:0x%x\r\n",virtualPort->pins[pin].function_code);

		switch (virtualPort->pins[pin].function_code) { /* handle current pin by function code */
		case PIN_DISABLED: /* pin is marked as disabled */
			*cur_DDR &= ~(1 << cur_pin); //disable PIN
			*cur_PORT &= ~(1 << cur_pin); //disable PULLUP
			break;
		case PIN_INPUT: /* pin marked as input -> set as input nothing else to to (value is readout @readIOpins() */
			*cur_DDR &= ~(1 << cur_pin); //set DDR "0"
			*cur_PORT |= (1 << cur_pin); //enable pullup PORT "1"
			break;
		case PIN_SWITCH: /* pin marked as output -> set as output and set the value of the specific bit of rxbuffer*/
			*cur_DDR |= (1 << cur_pin); /* set DDR "1" -> output */
			if (bis(instructions,(pin%8))) { //on or off
				*cur_PORT &= ~(1 << cur_pin); //set 0 -> "OFF"
			} else {
				*cur_PORT |= (1 << cur_pin); //set 1 -> "ON"
			}
			break;
		case PIN_PULSE: /* pin marked as output -> set as output and pulse the specific pin */
			*cur_DDR |= (1 << cur_pin); /* set DDR "1" -> output */
			if (bis(instructions,(pin%8))) { //only handle active pins
				*cur_PORT &= ~(1 << cur_pin); //set 0 > "OFF"
				_delay_ms(pulse_time); /* wait x ms */
				*cur_PORT |= (1 << cur_pin); //set 1 -> "ON"
			}
			break;
		case PIN_TOGGLE: /* pin marked as output -> set as output and pulse the specific pin */
			*cur_DDR |= (1 << cur_pin); /* set DDR "1" -> output */
			if (bis(instructions,(pin%8))) { //only handle active pins
				if (bis(*cur_PORT,(cur_pin%8))) { //test old value
					*cur_PORT &= ~(1 << cur_pin); //set OFF
				} else {
					*cur_PORT |= (1 << cur_pin); //set 1 -> "ON"
				}
			}
			break;
#ifdef USE_OW
		case PIN_OW_POWER_PARASITE:
			set1WirePin(&virtualPort->pins[pin], DS18X20_POWER_PARASITE);
			break;
		case PIN_OW_POWER_EXTERN:
			set1WirePin(&virtualPort->pins[pin], DS18X20_POWER_EXTERN);
			break;
#endif
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
			HANDLE_IO_DEBUG("unknown_func\r\n");
			*cur_DDR &= ~(1 << virtualPort->pins[pin].pin); //disable PIN
			*cur_PORT &= ~(1 << virtualPort->pins[pin].pin); //disable PULLUP
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

void initIOport(struct virtual_IO_port *virtualPort) {

	unsigned char *cur_DDR = 0; /* pointer to DDR register */
	unsigned char *cur_PORT = 0; /* pointer to O Port */
	unsigned char *cur_PIN = 0; /* pointer to I Pin */

	INIT_IO_DEBUG("\r\ninitIOport\r\n");

	for (int pin = 0; pin < VIRTUAL_PORT_PINCOUNT; pin++) { /* loop over all virtual IO Pins */
		INIT_IO_DEBUG("pin[%i]::",pin);
		cur_PORT = virtualPort->pins[pin].PPORT; /* get pointer to current port */
		INIT_IO_DEBUG("pPort:0x%x;",cur_PORT);

		cur_DDR = virtualPort->pins[pin].PDDR; /* get pointer to current datadirectionregister */
		INIT_IO_DEBUG("pDDR:0x%x;",cur_DDR);

		cur_PIN = virtualPort->pins[pin].PPIN; /* get pointer to current pin */
		INIT_IO_DEBUG("pPIN:0x%x;",cur_PIN);

		INIT_IO_DEBUG("pin:0x%x;",virtualPort->pins[pin].pin);

		INIT_IO_DEBUG("func:0x%x",virtualPort->pins[pin].function_code);

		if(cur_DDR == NULL || cur_PORT == NULL || cur_PIN == NULL){
			INIT_IO_DEBUG("-skip\r\n");
			continue;
		}else{
			INIT_IO_DEBUG("\r\n");
		}

		switch (virtualPort->pins[pin].function_code) { /* handle current pin by function code */
		case PIN_DISABLED: /* pin is marked as disabled */
			*cur_DDR &= ~(1 << virtualPort->pins[pin].pin); //disable PIN
			*cur_PORT &= ~(1 << virtualPort->pins[pin].pin); //disable PULLUP
			break;
		case PIN_INPUT: /* pin marked as input -> set as input nothing else to to (value is readout @readIOpins() */
			*cur_DDR &= ~(1 << virtualPort->pins[pin].pin); //set DDR "0"
			*cur_PORT |= (1 << virtualPort->pins[pin].pin); //enable pullup PORT "1"
			break;
		case PIN_PULSE: /* pin marked as output -> set as output and pulse the specific pin */
		case PIN_TOGGLE: /* pin marked as output -> set as output and pulse the specific pin */
		case PIN_SWITCH: /* pin marked as output -> set as output and set the value of the specific bit of rxbuffer*/
			*cur_DDR |= (1 << virtualPort->pins[pin].pin); /* set DDR "1" -> output */
			*cur_PORT |= (1 << virtualPort->pins[pin].pin); //set 1 -> "ON"
			break;
		case PIN_S0:
			break;
		default:
			*cur_DDR &= ~(1 << virtualPort->pins[pin].pin); //disable PIN
			*cur_PORT &= ~(1 << virtualPort->pins[pin].pin); //disable PULLUP
			break;
		}
	}
}

