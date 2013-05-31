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

#include <avr/io.h>
#include <avr/interrupt.h>

#include "yavrha_remote_relay.h"

/* Global Variables */
/* Buffer is used to load data to be send
	and to store the configuration data received */

uint8_t buffer[10];
/* This is a flag to be  used know that board
 is in waiting for configuration package		*/
uint8_t CONFIG_FLAG;

// GLOBAL Variables to store device status
// This example only use DATA0
uint8_t DATA0, DATA1, DATA2, DATA3;
// MSGID to send in txmode, received MSGID, received node number
uint8_t MSGID, RECV_MSGID;

// Function Prototypes
void relay_action(void);
void nrf_read_cfg_payload(void);
uint8_t nrf_read_payload(void);
void nrf_tx_config(void);
void nrf_send(uint8_t *);
void nrf_rx_config(void);
void report_status(void);
void spi_init(void);
void remote_nrf_config(void);

//          *** IMPORTANT ***
// Avoid nasty resets after wdt reset.
void wdt_init(void) __attribute__((naked)) __attribute__((section(".init3")));

// This interrupt function get call after reset to disable WDT
// This is critical to avoid infinites reset.
void wdt_init(void)
{
    MCUSR = 0;
    wdt_disable();

    return;
}

// This is the interrupt asserted after receiving a package

ISR(INT0_vect) {
	
	// Important disable Global interrupts
	cli();

    // This flag is used while waiting for configuration data.
	if (CONFIG_FLAG == 1)
	{
		nrf_read_cfg_payload();
		//clear flag
		CONFIG_FLAG = 0;
	} 

    
    else
	{
        // Check if received paylod belong to this node.		
        if ( nrf_read_payload() == 1 ) {
		
            relay_action();
            
            // Report Status Back.    
            report_status ();
        }
	}
	
	sei();
}


ISR(TIMER1_COMPA_vect)
{
			
		// Disable Interrupts		
	cli();
 
    // Report Current Status
    report_status();
    
    // Enable Interrupts
	sei();		
	
}

void relay_action() {
		
		if (DATA0 == 1)
		{
			PORTB |= (1<<PIN7);	
		} 

		else
		{
			PORTB &= ~(1<<PIN7);
		}	
}


void report_status () {

	// configure radio in TX
		nrf_tx_config();
		buffer[0] = DATA0;
		buffer[1] = DATA1;
		buffer[2] = DATA2;
		buffer[3] = DATA3;
		buffer[4] = MSGID++;
		buffer[5] =	NODE_NUMBER;
						
	// send package
		nrf_send(buffer);
						
	// Return radio to rx mode
		nrf_rx_config();

}

int main(void)
{
    // Relay output
	DDRB |= (1<<PB7);
	
	// Setup ISP
	spi_init();
	
	// Configure Push button pull-ups
	PORTD |= (1 << PD0);
	
	sei(); // Enable global interrupts
	
	// configure NRF parameters
    nrf_rx_config();
	
	// Enable Radio Interrupt
	EIMSK |= (1<<INT0);
   
    // Take NODE number from EEPROM to ram
    // This is to validate the incoming data
    NODE_NUMBER = eeprom_read_byte(&eeNODE_NUMBER);
   
    // The relay will report its status every 10s
    // Configure Timer/Counter 1 - 16 bits to 10s this assumes clock at 1Mhz
   
    // Set CTC compare value approx 10 sec at 1MHz AVR clock, with a prescaler of 1024
    OCR1A   = 9766;					
    
    // Configure timer 1 for CTC mode
    TCCR1B |= (1 << WGM12);		
   
    /*
    // Set CTC compare value to 1Hz at 1MHz AVR clock, with a prescaler of 64    
    OCR1A   = 15624;				

    // Start timer at Fcpu/64
    TCCR1B |= ((1 << CS10) | (1 << CS11)); 
    */
   
    // Start timer at Fcpu/1024
    TCCR1B |= ((1 << CS10) | (1 << CS12));
   
    // Enable CTC interrupt
    TIMSK1 |= (1 << OCIE1A); 
   
    while (1) {
      // Waiting for push button
	  
		 // If configuration push button is pressed
		 if (bit_is_clear(PIND,PD0)) {
			 
				// Configure radio to receive configuration
				remote_nrf_config();
				
				// Set Configuration Flag
				CONFIG_FLAG = 1;
				
				// Wait loop until push botton is released.
				while(bit_is_clear(PIND, PD0)){
			
				
				}
			
			// Reset MCU, using the wdt
			
				 wdt_enable (WDTO_1S,WDE); 
				 sei(); // Enable global interrupts
		 }
	
			// Send current status back after timer
				 				
    }
      
}
