/*
               Yavrha
     Copyright (C) Carlos Silva, 2013
      (csaleman [at] gmail [dot] com)

      http://code.google.com/p/yavrha/
*/

/*
    This file is part of Yavrha.

    Yavrha is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Yavrha is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Yavrha.  If not, see <http://www.gnu.org/licenses/>.

*/ 


#ifndef YAVRHA_REMOTE_RELAY_H_
#define YAVRHA_REMOTE_RELAY_H_


#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/eeprom.h>
#include <util/delay_basic.h>
#include <avr/pgmspace.h>
#include <avr/sfr_defs.h>
#include "nRF24L01.h"


/* Global Variables */
/* Buffer is used to load data to be send
	and to store the configuration data received */

extern uint8_t buffer[10];
extern uint8_t NODE_NUMBER;		// to store node number from ram.
extern uint8_t DATA0;			// To store device status & data
extern uint8_t DATA1;
extern uint8_t DATA2;
extern uint8_t DATA3;
extern uint8_t RECV_MSGID;			// To store the MSGID from incoming message
extern uint8_t RECV_NODE_NUMBER;	// To store the Node Number of received MSG.

// Define Radio nRF24L01 Radio Setting
#define PAYLOAD_WIDTH 6			// Must match RX {NODE#, MESS#, DATA3, DATA2, DATA1, DATA0}
#define CFG_PAYLOAD_WIDTH 7		// Number of bytes to receive {CH,NODE#,ADD4,ADD3,ADD2,ADD1,ADD0}

// not needed delete after testing. Node Address
extern uint8_t EEMEM eeNRF_ADDRESS[5];
extern uint8_t EEMEM eeNODE_NUMBER;
extern uint8_t EEMEM CH;


// Pin definitions for chip select chip enable and SPI
#define CSN  PB2
#define CE	 PB0
#define CLK  PB5
#define MOSI PB3
#define MISO PB4
#define IRQ	 PD2
#define PORT_IRQ PORTD
#define DDR_ISP DDRB
#define PORT_ISP PORTB


// Definitions for selecting and enabling the radio
#define CSN_HIGH      PORTB |=  (1<<CSN);
#define CSN_LOW       PORTB &= ~(1<<CSN);
#define CE_HIGH       PORTB |=  (1<<CE);
#define CE_LOW        PORTB &= ~(1<<CE);


// Since individual bits cannot be modified, all CONFIG settings must be set up at start up.
// EN_CRC enable CRC, CRCO set up 2 bytes CRC. As precaution, also MASK TX_DS and MAX_RT interrupt for RX
#define TX_POWERUP	  nrf_config_register(CONFIG,(1<<PWR_UP)|(1<<EN_CRC) | (1<<CRCO));
#define TX_POWERDWN  nrf_config_register(CONFIG, (1<<EN_CRC) | (1<<CRCO));
#define RX_POWERUP	  nrf_config_register(CONFIG,(1<<MASK_TX_DS)|(1<<MASK_MAX_RT)|(1<<PWR_UP)|(1<<EN_CRC) | (1<<CRCO) | (1<<PRIM_RX) );


// Watchdog Timer Definitions ***************************
#define 	WDTO_1S   6
#define 	WDTO_2S   7
#define 	WDTO_4S   32
#define 	WDTO_8S   33
#define 	wdt_reset()   __asm__ __volatile__ ("wdr")
// from wdt.h, modified for interrupt
// mode: Interrupt = WDIE
//		 Reset     = WDE
#define wdt_enable(value,mode)   \
__asm__ __volatile__ (  \
	"in __tmp_reg__,__SREG__" "\n\t"    \
	"cli" "\n\t"    \
	"wdr" "\n\t"    \
	"sts %0,%1" "\n\t"  \
	"out __SREG__,__tmp_reg__" "\n\t"   \
	"sts %0,%2" "\n\t" \
	: /* no outputs */  \
	: "M" (_SFR_MEM_ADDR(WDTCSR)), \
	  "r" (_BV(WDCE) | _BV(WDE)), \
	  "r" ((uint8_t) _BV(mode) | value) \
	: "r0"  \
)

#define wdt_disable() \
__asm__ __volatile__ (  \
	"in __tmp_reg__, __SREG__" "\n\t" \
	"cli" "\n\t" \
	"sts %0, %1" "\n\t" \
	"sts %0, __zero_reg__" "\n\t" \
	"out __SREG__,__tmp_reg__" "\n\t" \
	: /* no outputs */ \
	: "M" (_SFR_MEM_ADDR(WDTCSR)), \
	"r" ((uint8_t)(_BV(WDCE) | _BV(WDE))) \
	: "r0" \
)


// ********************************************





#endif /* YAVRHA_TEMP_SENDER_H_ */
