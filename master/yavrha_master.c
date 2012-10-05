/*
			Yavrha Project
			Carlos Silva, 2012
		
             LUFA Library
     Copyright (C) Dean Camera, 2011.

  dean [at] fourwalledcubicle [dot] com
           www.lufa-lib.org
*/


/** \file
 *
 *  Main source file for the VirtualSerial demo. This file contains the main tasks of
 *  the demo and is responsible for the initial application hardware configuration.
 */


#include "yavrha.h"

const char help_string[] PROGMEM = \
"stop \t\t\tstop debug continue output\n\
start \t\t\tstart debug continue output\n\
cfg_ch \t\t\t{0-83}\n\
cfg_home_addr\t\t{0-255},{0-255},{0-255},{0-255}\n\
enable_node \t\t{node#} {0-1}\n\
cfg_node_addr \t\t{node#} {0-255}\n\
cfg_node_name \t\t{node#} [10]\n\
cfg_node_type \t\t{0-5} {0-255}\n\
print_cfg \t\tPrint the configuration JSON\n\
reset \t\t\treset master configuration\n\
send_cfg \t\t{node#}\n\
get \t\t\tPrint JSON active nodes\n\
send \t\t\t{node#}, {0-255}, {0-255}, {0-255}, {0-255}\n\
help \t\t\tprint this help\n";

/* Function Prototypes: */
	void SetupHardware(void);
	void Print_RadioData(void);
	void Save_RadioData(void);
	void Cmd_Handler(char *);
	void print_cfg(void);
	void reset_radio(void);


// Global buffer, this is a extern global in others sources.
uint8_t buffer[10];
/* The only way I know to config array from header file.
 This is the address used to send configuration to nodes. */ 
uint8_t CFG_ADDR[] = CONFIGURATION_ADDR;

// Important this hold nodes received data.
uint8_t NodesData[MAXSNODES][PAYLOAD_WIDTH];
uint8_t PRINT_FLAG = 0;		// this is flag to enable continued data printout, just for debugging

// Global variable for Msgid
uint8_t MSGID;


/** LUFA CDC Class driver interface configuration and state information. This structure is
 *  passed to all CDC Class driver functions, so that multiple instances of the same class
 *  within a device can be differentiated from one another.
 */
USB_ClassInfo_CDC_Device_t VirtualSerial_CDC_Interface =
	{
		.Config =
			{
				.ControlInterfaceNumber   = 0,
				.DataINEndpoint           =
					{
						.Address          = CDC_TX_EPADDR,
						.Size             = CDC_TXRX_EPSIZE,
						.Banks            = 1,
					},
				.DataOUTEndpoint =
					{
						.Address          = CDC_RX_EPADDR,
						.Size             = CDC_TXRX_EPSIZE,
						.Banks            = 1,
					},
				.NotificationEndpoint =
					{
						.Address          = CDC_NOTIFICATION_EPADDR,
						.Size             = CDC_NOTIFICATION_EPSIZE,
						.Banks            = 1,
					},
			},
	};

/** Standard file stream for the CDC interface when set up, so that the virtual CDC COM port can be
 *  used like any regular character stream in the C APIs
 */
static FILE USBSerialStream;




ISR(INT0_vect) {
	
	cli();
	
	Save_RadioData();	// Save received data in structure
	
	if (PRINT_FLAG)		// Is verbose mode enable?
	{
	Print_RadioData();	// Print everyting in usb
	}	
		PORTE ^= (1<<PORTE6);	// toggle led.
		
	
	sei();
	
}


/** Main program entry point. This routine contains the overall program flow, including initial
 *  setup of all components and the main program loop.
 */
int main(void)
{
		char *CmdBuff = NULL;
		uint8_t strpos = 0;
		char temp;
		
	
	SetupHardware();
	spi_init();
	nrf_config();

	/* Create a regular character stream for the interface so that it can be used with the stdio.h functions */
	CDC_Device_CreateStream(&VirtualSerial_CDC_Interface, &USBSerialStream);

	sei();

	for (;;)
	{
		
	
				while(CDC_Device_BytesReceived(&VirtualSerial_CDC_Interface)){ 	
						
						temp = CDC_Device_ReceiveByte(&VirtualSerial_CDC_Interface);
					
					// Check if the character is a backspace
					if (temp == '\b' || temp == 127)
					{
						fputc((char)127,&USBSerialStream);   // send backspace back.
						
						// Since strpos is incremented at the end of the loop, I subtract 2
						// if is the last character strpos = 0, set it up to 255, it 
						// will be incremented to 0 at the end of the loop
							if (strpos >= 2) strpos = strpos - 2;
							else strpos = 255;						
						
					}
					else {
						CmdBuff = (char*) realloc(CmdBuff,1 * sizeof(char));
						CmdBuff[strpos] = temp;
						fputc(CmdBuff[strpos],&USBSerialStream);
						}
					
						if(CmdBuff[strpos]== '\n' || CmdBuff[strpos]== '\r') {
																
								CmdBuff[strpos] = '\0';
								
								fputc('\n',&USBSerialStream);
								Cmd_Handler(CmdBuff);
	
								free(CmdBuff);
								strpos = 0;
								break;
								}	
								
								strpos++;
								
						}		// end of while
						
		
		CDC_Device_USBTask(&VirtualSerial_CDC_Interface);
		USB_USBTask();
	}
}

// THIS IS THE MAIN COMMAND HANDLER FUNCTION THIS IS JUST A LOT OF MESSY CODE
void Cmd_Handler( char *CmdBuff) {
		
		char *token;
		uint8_t temp,temp1,i,k;
		uint8_t temp_array[10];
		uint8_t ram_NRF_ADDR[5];
		const char delimiter[] = " ,"; // token delimiter is just one space or coma.
		// array to store the message to be send to node
		uint8_t msg_to_send[PAYLOAD_WIDTH];
						
	
	
	
		token = strtok(CmdBuff,delimiter);
	
					//  Stop Printing data continuously.
					if(strcmp(token,"stop")== 0) {
						PRINT_FLAG = 0;
						fprintf(&USBSerialStream, "stoped\n");
								
						}
						
					//	Start Printing data continuously.
					else if(strcmp(token,"start")== 0) 	{
								
						PRINT_FLAG = 1;
						fprintf(&USBSerialStream, "started\n");
						}							 
/* ******************************************************** */					
					//	Store Radio Channel in eeprom.
						
						else if(strcmp(token,"cfg_ch")== 0) 	{
								
							if ((token = strtok(NULL,delimiter)))	{
								
									temp = atoi(token);
									// check if the channel is valid in US
									if (temp >0 && temp < 84)
									{
										eeprom_write_byte(&CH,temp);	
										
										//reconfigure radio.
										nrf_config();
										
										fprintf(&USBSerialStream, "Home Channel: %i \n",temp);
									}	
										
							}								
									
						}
/* ******************************************************** */							
						//	Store Radio Home Address in eeprom.
						
						else if(strcmp(token,"cfg_home_addr")== 0) 	{
							
							i=0;  // initialize i before start while
							
						
								
							while ((token = strtok(NULL,delimiter)))	{
								
									temp = atoi(token);
									
									eeprom_write_byte((uint8_t *)&eeNRF_ADDRESS+i,temp);
								
									i++;
						
							}				
							//reconfigure radio.
										nrf_config();
										fprintf(&USBSerialStream, "\nHome Address");
										for(i=0; i<5; i++){
											fprintf(&USBSerialStream, ":%i",eeprom_read_byte((uint8_t *)&eeNRF_ADDRESS+i));
											
										}
									fprintf(&USBSerialStream, " \n");	
						}						
/* ******************************************************** */							
						//	Enable Node.
						
						else if(strcmp(token,"enable")== 0) 	{
							// this token get the node number 0-MAXNODES	
							if ((token = strtok(NULL,delimiter)))	{
								
									temp = atoi(token);
									// check for valid node number
									if (temp >= 0 && temp < MAXSNODES)
									{
										Nodes[temp].NodeEnable = 1;	
										NodeStrucToEeprom();
										fprintf(&USBSerialStream, "node enabled: %i\n",temp);
										nrf_config();								
									
									}
							}						
						}  // else if
/* ******************************************************** */	
						//	Disable Node.
						
						else if(strcmp(token,"disable")== 0) 	{
							// this token get the node number 0-MAXSNODES	
							if ((token = strtok(NULL,delimiter)))	{
								
									temp = atoi(token);
									// check for valid node number
									if (temp >= 0 && temp < MAXSNODES)
									{
										Nodes[temp].NodeEnable = 0;	
										NodeStrucToEeprom();
										fprintf(&USBSerialStream, "node disabled: %i\n",temp);
										nrf_config();								
									
									}
							}						
						}  // else if```````````````````````````````````````````````````````````````````````````````````````````````````````````````````		

/* ******************************************************** */					
						//	Save Node Name
						
						else if(strcmp(token,"cfg_node_name")== 0) 	{
							// this token get the node number 0-MAXSNODES	
							
							if ((token = strtok(NULL,delimiter)))	{
								
									temp = atoi(token);
									// check for valid node number
									if (temp >= 0 && temp < MAXSNODES)
									{
											
											if ((token = strtok(NULL,delimiter)))	{
													strlcpy(Nodes[temp].NodeName,token,sizeof(Nodes[temp].NodeName));	
													NodeStrucToEeprom();
													fprintf(&USBSerialStream, "node name: %i %s \n",temp, Nodes[temp].NodeName);
																							
											}								
									
									}
							}						
						}  // else if

/* ******************************************************** */					
						//	Save Node address
						
						else if(strcmp(token,"cfg_node_addr")== 0) 	{
							// this token get the node number 0 - MAXNODES	
							if ((token = strtok(NULL,delimiter)))	{
								
									temp = atoi(token);
									// check for valid node number
									if (temp >= 0 && temp < MAXSNODES)
									{
												// get the node address 0 to 255
											if ((token = strtok(NULL,delimiter)))	{
								
												temp1 = atoi(token);
												
												if (temp1 >= 0 && temp1 <= 255)
													{
										
														Nodes[temp].NodeAddr = temp1;	
														NodeStrucToEeprom();
														fprintf(&USBSerialStream, "node address: %i %i\n",temp,temp1);
														nrf_config();
													
													}													
										
											}								
									
									}
							}						
						}  // else if		
		/* ******************************************************** */					
						//	Save Node type //
					
						
						else if(strcmp(token,"cfg_node_type")== 0) 	{
							// this token get the node number 0-MAXNODES	
							if ((token = strtok(NULL,delimiter)))	{
								
									temp = atoi(token);
									// check for valid node number
									if (temp >= 0 && temp < MAXSNODES)
									{
												// get the node type 0 to 255
											if ((token = strtok(NULL,delimiter)))	{
								
												temp1 = atoi(token);
												
												if (temp1 >= 0 && temp1 <= 255)
													{
										
														Nodes[temp].NodeType = temp1;	
														NodeStrucToEeprom();
														fprintf(&USBSerialStream, "node type: %i %i\n",temp,temp1);
														nrf_config();
													
													}													
										
											}								
									
									}
							}						
						}  // else if		
						
		/* ******************************************************** */					
						//	Print Configuration //
					
						
						else if(strcmp(token,"print_cfg")== 0) 	{
								
								print_cfg();	
						}							
		/* ******************************************************** */					
						//	Reset Radio configurations //
					
						
						else if(strcmp(token,"reset")== 0) 	{
								
								reset_radio();	
						}							
						
		/* ******************************************************** */					
						//	Print Nodes Data//
					
						
						else if(strcmp(token,"get")== 0) 	{
								
								Print_RadioData();	
						}							
						
		/* ******************************************************** */
						//	Send Data to RX Node //
						
						else if(strcmp(token,"send")== 0) 	{
								
								if ((token = strtok(NULL,delimiter)))	{
								
									temp = atoi(token);
									// check for valid node number
									if (temp >= 0 && temp < MAXSNODES)
									{
										
										// clear temporary array and variables
										i=0;
										
										for (k=0;k<PAYLOAD_WIDTH;k++) {
											
											msg_to_send[k]=0;
										}
										
										// Get the data to be send
										/*	
											This loop stay until the last value is saved or the maximum
											number of values is entered, in this case 4 bytes.
											{NODE#, MSGID#, DATA3, DATA2, DATA1, DATA0}
										*/		
										while ((token = strtok(NULL,delimiter)) && (i < PAYLOAD_WIDTH-2))	{
											temp1 = atoi(token);
											msg_to_send[i] = temp1 >=0 && temp1 <=255 ? temp1 : 0;
											i++;
										}		
										
										// Must match TX  {NODE#, MSGID#, DATA3, DATA2, DATA1, DATA0}
										// Node Number
										msg_to_send[PAYLOAD_WIDTH-1] = temp;
										// MSGID
										msg_to_send[PAYLOAD_WIDTH-2] = MSGID++;
										
										// Load eeNRF_ADDRESS from EEPROM into ramNRF_ADDR
										eeprom_read_block((void *)&ram_NRF_ADDR, (const void *)&eeNRF_ADDRESS,5);
										//get node address
										ram_NRF_ADDR[4] = Nodes[temp].NodeAddr;
										
										nrf_TXnodeCfg(eeprom_read_byte(&CH),ram_NRF_ADDR);	// Configure radio with ch and rx node address
											
											// Send Data to Node X
											if(send_node_data(msg_to_send) == 1) 		
												{
													fprintf(&USBSerialStream, "Data Sent Successfully\n");
												}
											else {
											
													fprintf(&USBSerialStream, "Data Sent Failed\n");
												}
										nrf_config();				// Return radio to RX mode
									}
								}																			
						}										
						
			
		/* ******************************************************** */					
						//	Send Node Configuration //
					
						
						else if(strcmp(token,"send_cfg")== 0) 	{
								
								if ((token = strtok(NULL,delimiter)))	{
								
									temp = atoi(token);
									// check for valid node number
									if (temp >= 0 && temp < MAXSNODES)
									{
										
										nrf_TXnodeCfg(CFG_CH, CFG_ADDR);			// Configure radio with default addr and channel
											
											
											// Send configuration to Node X
											if(send_node_cfg(temp) == 1) 		
												{
													fprintf(&USBSerialStream, "Configuration Sent Successfully\n");
												}
											else {
											
													fprintf(&USBSerialStream, "Configuration Sent Failed\n");
												}
										nrf_config();				// Return radio to RX mode
									}
								}																			
						}							
	
		/* ******************************************************** */						
						//	Print Help  //
						
						else if(strcmp(token,"help")== 0) 	{
							
							fprintf_P(&USBSerialStream,help_string);
									
													
						}  // else if
		/* ******************************************************** */	
						
						
						
} // end of function

// END OF COMMAND HANDLER


// Print configuration
void print_cfg() {
	
	uint8_t ram_NRF_ADDR[5];
	uint8_t channel;
	uint8_t i;
	
	
	channel = eeprom_read_byte(&CH);
	// LOAD eeNRF_ADDRESS from EEPROM into ramNRF_ADDR
	eeprom_read_block((void *)&ram_NRF_ADDR, (const void *)&eeNRF_ADDRESS,5);
	
	fprintf(&USBSerialStream, "{\"channel\":%i,\"home_addr\":[%i, %i, %i, %i],\n",channel,ram_NRF_ADDR[0],ram_NRF_ADDR[1],ram_NRF_ADDR[2],ram_NRF_ADDR[3]);
	
	for (i=0;i<MAXSNODES -1;i++)
	{
	fprintf(&USBSerialStream, "\"node%i\": {\"number\":%i, \"enable\":%i, \"address\":%i, \"name\":\"%s\", \"type\":%i},\n",
				i, Nodes[i].Node_Number, Nodes[i].NodeEnable,Nodes[i].NodeAddr,Nodes[i].NodeName,Nodes[i].NodeType);	
	}
	// Last node without trailing comma
	fprintf(&USBSerialStream, "\"node%i\": {\"number\":%i, \"enable\":%i, \"address\":%i, \"name\":\"%s\", \"type\":%i}\n",
			MAXSNODES-1, Nodes[MAXSNODES-1].Node_Number, Nodes[MAXSNODES-1].NodeEnable, Nodes[MAXSNODES-1].NodeAddr, Nodes[MAXSNODES-1].NodeName, Nodes[MAXSNODES-1].NodeType);	
	
	fprintf(&USBSerialStream,"}\n");
	
}
// Reset or initialize radio values in Nodes structure.
void reset_radio(){
	
	uint8_t i;
	
	eeprom_write_byte(&CH,0);
	
	for (i=0; i < MAXSNODES; i++)	// initialize nodes' config structure	
	{
		Nodes[i].NodeEnable = 0;
		Nodes[i].Node_Number = i;
		Nodes[i].NodeAddr = 0;
		Nodes[i].NodeName[0] = '\0';
		Nodes[i].NodeType = 0;
	}
	
	NodeStrucToEeprom();
}


// Save received data in global variable NodesData after interrupt
void Save_RadioData(void){
	
	uint8_t i;
	
	nrf_read_payload();
	
	for (i=0; i < PAYLOAD_WIDTH; i++)
	{
		
		NodesData[buffer[PAYLOAD_WIDTH-1]][i] = buffer[i];
		
	}
	
	
}

// Print the radio data in a JSON string.
void Print_RadioData(void) {
		
		uint8_t i, k;
		uint8_t firstitem = 0;
	
	fprintf(&USBSerialStream,"{ \n");
	for (i = 0; i < MAXSNODES; i++){
		
		if (Nodes[i].NodeEnable == 1)
		{
			
			// this is my way to get rid of trailing comma in JSON
			if (firstitem == 0)
				{
				// first element without comma	
				fprintf(&USBSerialStream,"\"node%i\": {",i);
				firstitem =1;	
				}
			// following elements with comma in front	
			else fprintf(&USBSerialStream,", \n\"node%i\": {",i);
			
			// print node number, reduntant.
			//fprintf(&USBSerialStream,"\"node\":%i, ", NodesData[i][PAYLOAD_WIDTH - 1]);
			
			// print message id
			fprintf(&USBSerialStream,"\"msgid\":%i, ", NodesData[i][PAYLOAD_WIDTH - 2]);
			
			for (k=PAYLOAD_WIDTH - 3;k > 0 ; k--) {
					
				fprintf(&USBSerialStream,"\"data%i\":%i, ",k,NodesData[i][k]);
			}
				// Print the last value without the trailing comma.
				
				fprintf(&USBSerialStream,"\"data0\":%i }",NodesData[i][0]);
		}
		
		
	}		
	fprintf(&USBSerialStream,"\n }\n");
	
}




/** Configures the board hardware and chip peripherals for the demo's functionality. */
void SetupHardware(void)
{
	/* Disable watchdog if enabled by bootloader/fuses */
	MCUSR &= ~(1 << WDRF);
	wdt_disable();

	/* Disable clock division */
	clock_prescale_set(clock_div_1);

	/* Hardware Initialization */
	USB_Init();
	
	// Set PORTE6 LED in ADAFRUIT Board to Output
	DDRE |= (1<<PORTE6);
	
}

/** Event handler for the library USB Connection event. */
void EVENT_USB_Device_Connect(void)
{
	
}

/** Event handler for the library USB Disconnection event. */
void EVENT_USB_Device_Disconnect(void)
{
	
}

/** Event handler for the library USB Configuration Changed event. */
void EVENT_USB_Device_ConfigurationChanged(void)
{

	CDC_Device_ConfigureEndpoints(&VirtualSerial_CDC_Interface);
	
}

/** Event handler for the library USB Control Request reception event. */
void EVENT_USB_Device_ControlRequest(void)
{
	CDC_Device_ProcessControlRequest(&VirtualSerial_CDC_Interface);
}

/* Important. this event MUST be used to avoid the blocking timeouts with the routines */
/* Do not try to send data if the DTR line (Host, aka. Terminal client) is not ready to receive */

void EVENT_CDC_Device_ControLineStateChanged(USB_ClassInfo_CDC_Device_t* const CDCInterfaceInfo)
{
        bool CurrentDTRState = (CDCInterfaceInfo->State.ControlLineStates.HostToDevice & CDC_CONTROL_LINE_OUT_DTR);
 
        if (CurrentDTRState)
        {
				// Enable radio data Printout once client is ready. 
				// PRINT_FLAG = 1;
                
				
        }
        else
        {
				// Disable radio interrupt if host is not ready, or no terminal is listening.
				PRINT_FLAG = 0;
				
        }
}