/*
	    Yavrha Project
	  Carlos Silva, 2012
		
             LUFA Library
     Copyright (C) Dean Camera, 2011.

  dean [at] fourwalledcubicle [dot] com
           www.lufa-lib.org
*/


#ifndef YAVRHA_H_
#define YAVRHA_H_

/*
 *
 *  Header file for VirtualSerial.c.
 */


	/* Includes: */
		#include <avr/io.h>
		#include <avr/wdt.h>
		#include <avr/power.h>
		#include <util/delay_basic.h>
		#include <avr/interrupt.h>
		#include <stdlib.h>
		#include <avr/eeprom.h>
		#include "Descriptors.h"
		#include <LUFA/Version.h>
		#include <LUFA/Drivers/USB/USB.h>
		#include <avr/pgmspace.h>
	
	/* LUFA Function Prototypes: */
		void SetupHardware(void);
		void CheckJoystickMovement(void);

		void EVENT_USB_Device_Connect(void);
		void EVENT_USB_Device_Disconnect(void);
		void EVENT_USB_Device_ConfigurationChanged(void);
		void EVENT_USB_Device_ControlRequest(void);




	/* Struct Declaration */
	/* This is the structure to hold nodes Configurations */
		typedef struct{
		uint8_t NodeEnable;
		uint8_t Node_Number;
		uint8_t NodeAddr;
		char NodeName[15];			// Only 15 bytes or less to conserve EEPROM 
		uint8_t NodeType;			// 1 for temp; 2 for Water alarm; 3 for switch. etc.
		}tNodesData;				// New struct Type.
		
	/* Common Definitions */
		#define PAYLOAD_WIDTH 6			// Must match TX  {NODE#, MSGID#, DATA3, DATA2, DATA1, DATA0}
		
		
	/*	Define the number of nodes supported; Nodes 0 to 5 are used to configure
		radio PIPE-0 to PIPE-5, additional nodes if used can share these channels.
		Use any number from 6 to 50, 50 nodes will used 93% of EEPROM and %78 of RAM
	*/	 
		
		#define MAXSNODES 12				
	// **************************
		
		#define CFG_PAYLOAD_WIDTH 7		// Number of configuration bytes to send: {CH,NODE#,ADD4,ADD3,ADD2,ADD1,ADD0}
		#define CFG_CH	2				// channel used to send configurations
		#define CONFIGURATION_ADDR {0xE7,0xE7,0xE7,0xE7,0xE7}	//address used to send configuration to nodes

	
	/* Function Prototypes: */

		extern void spi_init(void);
		extern void nrf_config(void);
		extern void nrf_read_register(uint8_t, uint8_t *, uint8_t);
		extern void nrf_config_register(uint8_t, uint8_t);
		extern uint8_t nrf_read_payload(void);
		extern void stopRadio(void);
		extern void startRadio(void);
		extern void NodeStructToRam(void);
		extern void NodeStrucToEeprom(void);
		extern void nrf_TXnodeCfg(uint8_t, uint8_t * );
		extern uint8_t send_node_cfg(uint8_t);
		extern uint8_t send_node_data(uint8_t *);
		
	/* EEPROM AND GLOBAL VARIABLES */
		extern uint8_t EEMEM CH;
		extern uint8_t EEMEM eeNRF_ADDRESS[4];		//hold 4 MSB of 5
		extern tNodesData EEMEM eeNodes[MAXSNODES];
		
		// Buffer MUST be bigger than PAYLOAD_WIDTH in receiver;
		// At least 5 to store addresses.
		extern uint8_t buffer[10];
		extern tNodesData Nodes[MAXSNODES];




#endif /* YAVRHA_H_ */
