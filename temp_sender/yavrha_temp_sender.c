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

#include "yavrha_temp_sender.h"

/* Global Variables */
/* Buffer is used to load data to be send
	and to store the configuration data received */

uint8_t buffer[10];
uint8_t NODE_NUMBER;

// Function Prototypes
void getdata(void);
void wdt_init(void) __attribute__((naked)) __attribute__((section(".init3")));


// This interrupt function get call after reset to disable WDT
// This is critical to avoid infinites reset.
void wdt_init(void)
{
    MCUSR = 0;
    wdt_disable();

    return;
}

// This is the watchdog interrupt vector

ISR(WDT_vect) {
   
		getdata();
		
	 nrf_send(buffer);
   
}


ISR(INT0_vect) {
	
	// Important disable Global interrupts
	cli();
	nrf_read_payload();
	
}



void getdata()
	{
		int temp;
		temp = ADC_temperature();
		
		buffer[0] = (temp & 0xFF);		// store 8 LSB in buffer[0], this is the temperature 
		buffer[1] = (temp >> 8);		// store 8 MSB in buffer[1], only bit 0 is used for temp. sign.
		buffer[2] += 0;
		buffer[3] += 0;
		buffer[4] += 1;
		buffer[5] = NODE_NUMBER;
	
	}



int main(void)
{
    // Set up CE as output
    DDRB = (1<<CE);
	
	// Setup ISP
	spi_init();
 
	// configure ADC
	ADC_init();
    
	// Configure Push button pull-ups
	PORTD |= (1 << PD0);
	
  // Enable Watchdog timer interrupt  
   wdt_enable (WDTO_8S,WDIE); 
  
   sei(); // Enable global interrupts
   
   // Use the Power Down sleep mode
   set_sleep_mode(SLEEP_MODE_PWR_DOWN);
 
	// Load NODE Number from EEPROM to RAM 
 
	NODE_NUMBER = eeprom_read_byte(&eeNODE_NUMBER);
	
// configure NRF parameters
   nrf_config();
   
	 
   while (1) {
      // Waiting for watchdog interrupt... 
	  
		 // If configuration push button is pressed
		 if (bit_is_clear(PIND,PD0)) {
			 
				// Disable WDT Interrupt
				wdt_disable();
			 
				// Configure radio to receive configuration
				remote_nrf_config();
				
				// Wait loop until push botton is released.
				while(bit_is_clear(PIND, PD0)){
			
				
				}		// loop to wait for wathdog timer reset.
			
			// Reset MCU, using the wdt
			
				 wdt_enable (WDTO_1S,WDE); 
				 sei(); // Enable global interrupts
		 }
		 
		 else {

			sleep_mode();   // go to sleep and wait for WDT interrupt...
		 }
		 		
   }
}
