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


// Function Prototypes

void ADC_init(void);
int ADC_read(void);
int ADC_temperature(void);



/* 	Positive Celcius Temperature Tables 
	Positive Celcius temperatures (ADC-value)
	
 */
const int TEMP_Celcius_pos[] PROGMEM =  
// Sensor:  NTCS0805E3104JXT VISHAY  100K
{											// from 0 to 60 degrees
    791,781,771,761,751,741,730,720,709,698,687,675,664,652,641, //0-14
    629,618,606,594,582,571,559,547,535,524,512,500,489,478,466, //15-29
    455,444,433,422,412,401,391,381,371,361,351,342,333,323,314, //30-44
    306,297,289,281,273,265,257,250,243,236,229,222,216,209,203, //45-59
    197,                                                         //60
};
#define TEMP_Celcius_pos_entries	61    // Number of entries in the positive lookup table
#define TEMP_ADC_max				908   // ADC Value at lowest temperature
#define TEMP_ADC_min				197   // ADC Value at highest temperature
#define TEMP_ADC_zero				791   // ADC Value at zero degrees C


/* 
 *	Negative Temperature Tables
 *
 *	Repeating data for the zero point in these tables makes the code smaller
 *	so it is worth repeating an extra 2 bytes of data from the positive tables to 
 *	make a 32 byte saving overall.
 */
const int TEMP_Celcius_neg[] PROGMEM =    // Negative Celcius temperatures (ADC-value)

// Sensor:  NTCS0805E3104JXT VISHAY  100K
{											// from 0 to -15 degrees
    791,														 // 0
	800,810,818,827,836,844,852,860,867,875,882,889,895,902,908, // -1..-15   
};
#define TEMP_Celcius_neg_entries	16		// Number of entries in the negative lookup table


void ADC_init(void)  
{

    ADCSRB = 0; // ADC free running mode
    

    ADCSRA = (1<<ADEN) | (1<<ADPS2) | (1<<ADPS0); 

		// external AREF for ADC, switch off internal reference
        ADMUX = (1<<REFS0);
		
		// set ADC prescaler to , 1MHz / 8 = 125kHz
		ADCSRA = (1<<ADPS1) | (1<<ADPS0); 
                                        
		
		// Select ADC Channel ADC1
		ADMUX |= (1<<MUX0);      
        
        // Enable the VCP (VC-peripheral)

		DDRC |=  (1<<DDC0); // sbi(DDRC, PORTC0);  		
		 
}


/*****************************************************************************
*
*   Function name : ADC_read
*
*   Returns :       int ADC average value after 8 reads
*
*   Parameters :    None
*
*   Purpose :       Do a Analog to Digital Conversion
*              
*
*****************************************************************************/
int ADC_read(void)
{
    char i;
    int ADC_temp;
    
    // To save power, the voltage over the LDR and the NTC is turned off when not used
    // This is done by controlling the voltage from a I/O-pin (PORTC0)
    // The VCP should be disabled if the internal reference is being used
    
        // Enable the VCP (VC-peripheral)
        PORTC |=(1<<PC0);
		
		// Enable ADC
		ADCSRA |= (1<<ADEN);
        
		ADCSRA |= (1<<ADSC);            // do single conversion		                      
		while(ADCSRA & (1<<ADSC));          // wait for conversion done, ADSC flag to zero
	
	ADC_temp = ADCL;                // read out ADCL register. 
    ADC_temp += (ADCH << 8);        // read out ADCH register.
    
	
    // disable the VCP (VC-peripheral)
    PORTC &= ~(1<<PC0);
  
    
    // disable the ADC: 
    ADCSRA &= ~(1<<ADEN); 
	
    //return Temperature;
	return ADC_temp;
	
}

/*****************************************************************************
*
*   Function name : ADC_temperature
*
*   Returns :       return temperature in Celsius, sign in bit 8
					if bit 8 is 0 temperature is negative, 1 if positive
*
*   Parameters :    None
*
*   Purpose :      Perform lookup table search
*              
*	TODO :			This function don't handle temp limits.
*					Be careful with temp below -15 or above 60 C. 
*
*****************************************************************************/
int ADC_temperature(void)
{
	int ADCresult;				// Result from ADC conversion
	uint8_t i;					// index variable for loops
	uint8_t temperature_int;		// integer portion of the temperature
	int	temperature;			// nine bit sign, and bit 0-8 temperature
	uint8_t sign;				// Sign flag, 0 for negative and 1 for positive temp.
		

	ADCresult = ADC_read();
	//ADCresult += 200;			// just to test negative functions

	/*	Convert the raw ADC value into a temperature using the lookup tables for  
		the integer portion of the temperature.
		The table index (i) is the integer portion of the temperature at the 
		point where the for loop exits via a break instruction.
	*/
   if(ADCresult >  TEMP_ADC_zero){              // it's a negative temperature
   		sign = 0;								// Set the sign
		
		// Find the temperature in the negative table
		for (i=1; i < TEMP_Celcius_neg_entries; i++){  
			
			// if the value matches exactly the table entry then no decimal
			// portion needs to be calculated.
			if(ADCresult == pgm_read_word(&TEMP_Celcius_neg[i]) ){
				temperature_int = i;		// integer portion is the table entry index
				break;							// stop scanning through the table
			
			}
			else if (ADCresult < pgm_read_word(&TEMP_Celcius_neg[i])){
				temperature_int =  i-1;			// integer portion is one less than table entry reached
				break;							// stop scanning through the table
			}
			
		}
    } else if (ADCresult < TEMP_ADC_zero ){		// it's a positive temperature
		sign = 1;								// Set the sign 
        
		// Find the temperature in the positive table
        for (i=1; i < TEMP_Celcius_pos_entries; i++)  {   
			
			// if the value matches the table entry exactly then no decimal 
			// portion needs to be calculated.
			if (ADCresult == pgm_read_word(&TEMP_Celcius_pos[i]) ){ 
				temperature_int = i - 1;		// integer portion is one less than table entry reached
				break;							// stop scanning through the table	
            }
			else if (ADCresult > pgm_read_word(&TEMP_Celcius_pos[i])){ 
				temperature_int = i-1;			// integer portion is one less than table entry reached
				break;							// stop scanning through the table
            }
			
        }        
    
	} else {									// the temperature is exactly zero degrees in the table
		
		temperature_int = 0;
		sign = 1;								// This is just to say that zero C is positive; 
												// user implementation will not show + sign just - temp.
    }
    
	// Store sign in ninth bit
	temperature = (sign << 8);
	temperature += temperature_int;
	
	return temperature;
}
