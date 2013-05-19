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

#include "yavrha_LED_Dimmer.h"

/* Global Variables */
/* Buffer is used to load data to be send
	and to store the configuration data received */

uint8_t buffer[10];
/* This is a flag to be  used know that board
 is in waiting for configuration package		*/
uint8_t CONFIG_FLAG;

// GLOBAL Variables to store device status
// This example only use DATA0 and DATA1
// DATA0 is 1 for ON, 0 for OFF, DATA1 is 0 to 255 for PWM.
uint8_t DATA0, DATA1, DATA2, DATA3;
// MSGID to send in txmode, received MSGID, received node number
uint8_t MSGID, RECV_MSGID;

// Function Prototypes
void relay_action(void);
void nrf_read_cfg_payload(void);
void nrf_read_payload(void);
void nrf_tx_config(void);
void nrf_send(uint8_t *);
void nrf_rx_config(void);
void pwm_init(void);

//          *** IMPORTANT ***
// Avoid nasty resets after wdt reset.
void wdt_init(void) __attribute__((naked)) __attribute__((section(".init3")));

// This interrupt function get call after reset to disable WDT
// This is critical to avoid infinites resets.
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
	if (CONFIG_FLAG == 1)
	{
		nrf_read_cfg_payload();
		//clear flag
		CONFIG_FLAG = 0;
	} 
	else
	{
		nrf_read_payload();
		relay_action();
	}
	
	sei();
}


ISR(TIMER1_COMPA_vect)
{
			
	cli();
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
		sei();	
	
}

// This function initialize the 8bit Timer/Counter 0 as a PWM

void pwm_init() {

    // Start timer 0 at Fcpu/64
//   TCCR0B |= (1 << CS00) | (1 << CS01); 

    // Start timer 0 at Fcpu/8
    TCCR0B |= (1 << CS01); 


    // Configure timer 0 for PWM mode
    TCCR0A |= (1 << WGM00) | (1 << COM0A1);		
    
    // make sure to make OC0A pin (pin PD6 for atmega8 Series)
    DDRD |= (1<<PD6);

    // Default 50% Duty Cycle for Debugging
    OCR0A = 127;
}

// This function change the relay output state and modified the PWM Duty Cycle
void relay_action() {
		
		if (DATA0 == 1)
		{
            // Relay On			
            PORTB |= (1<<PIN7);	
            // Change PWM Duty Cycle PD6
            OCR0A = DATA1;
		} 

		else
		{
            // Relay Off			
            PORTB &= ~(1<<PIN7);
            // PWM to 0%
            OCR0A = 0;

		}	
}


int main(void)
{
    // Relay output
	DDRB |= (1<<PB7);
	
	// Setup ISP
	spi_init();
	
	// Configure Push button pull-ups
	PORTD |= (1 << PD0);
	
    // Enable global interrupts
    sei();
	
	// configure NRF parameters
    nrf_rx_config();
	
	// Enable Radio Interrupt
	EIMSK |= (1<<INT0);
    
    // Take NODE number from EEPROM to ram
    // This is to validate the incoming data
    NODE_NUMBER = eeprom_read_byte(&eeNODE_NUMBER);

    // Initialize Timer/Counter 0 8bit as PWM
    pwm_init();

    // The relay will report its status every 10s
    // Configure Timer/Counter 1 - 16 bits to 10s this assumes clock at 1Mhz    

    // Set CTC compare value approx 10sec at 1MHz AVR clock, with a prescaler of 1024
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
			 
				// Configure radio to receive configuration parameters
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
