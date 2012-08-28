/*
 * Cmd_Handler.c
 *
 * Created: 2/11/2012 7:13:15 PM
 *  Author: Carlos.
 */ 

#include "yavrha.h";

// THIS IS THE MAIN COMMAND HANDLER FUNCTION THIS IS JUST A MESSY CODE
void Cmd_Handler( char *CmdBuff) {
		
		char *token;
		uint8_t temp,temp1,i;
		const char delimiter[] = " ,"; // token delimiter is just one space or coma. 
	
	
	
		token = strtok(CmdBuff,delimiter);
	
					//  Stop Receiving. Just disable Interrupt INT0
					if(strcmp(token,"stop")== 0) {
						EIMSK &= ~(1<<INT0); // stop interrupt
						fprintf(&USBSerialStream, "stop\n\r");
								
						}
						
					//	Start Receiving. Just Enable Interrupt INT0	
					else if(strcmp(token,"start")== 0) 	{
								
						EIMSK |= (1<<INT0); // start interrupt
						fprintf(&USBSerialStream, "start\n\r");
						}							 
				
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
										
										fprintf(&USBSerialStream, "Home Channel: %i \n\r",temp);
									}	
										
							}								
									
						}
						
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
									fprintf(&USBSerialStream, " \n\r");	
						}						
						
						//	Enable or Disable nodes.
						
						else if(strcmp(token,"enable_node")== 0) 	{
							// this token get the node number 0-5	
							if ((token = strtok(NULL,delimiter)))	{
								
									temp = atoi(token);
									// check for valid node number
									if (temp >= 0 && temp <= 5)
									{
												// get a 0 to disable or a 1 to enable
											if ((token = strtok(NULL,delimiter)))	{
								
												temp1 = atoi(token);
												
												if (temp1 == 0 || temp1 == 1)
													{
										
														Nodes[temp].NodeEnable = temp1;	
														NodeStrucToEeprom();
														fprintf(&USBSerialStream, "node enable: %i %i\n\r",temp,temp1);
														nrf_config();
													
													}													
										
											}								
									
									}
							}						
						}  // else if						

/* ******************************************************** */					
						//	Save Node Name
						
						else if(strcmp(token,"cfg_node_name")== 0) 	{
							// this token get the node number 0-5	
							
							if ((token = strtok(NULL,delimiter)))	{
								
									temp = atoi(token);
									// check for valid node number
									if (temp >= 0 && temp <= 5)
									{
											
											if ((token = strtok(NULL,delimiter)))	{
													strlcpy(Nodes[temp].NodeName,token,sizeof(Nodes[temp].NodeName));	
													NodeStrucToEeprom();
													fprintf(&USBSerialStream, "node name: %s \n\r",Nodes[temp].NodeName);
																							
											}								
									
									}
							}						
						}  // else if

/* ******************************************************** */					
						//	Save Node address
						
						else if(strcmp(token,"cfg_node_addr")== 0) 	{
							// this token get the node number 0-5	
							if ((token = strtok(NULL,delimiter)))	{
								
									temp = atoi(token);
									// check for valid node number
									if (temp >= 0 && temp <= 5)
									{
												// get the node address 0 to 255
											if ((token = strtok(NULL,delimiter)))	{
								
												temp1 = atoi(token);
												
												if (temp1 >= 0 && temp1 <= 255)
													{
										
														Nodes[temp].NodeAddr = temp1;	
														NodeStrucToEeprom();
														fprintf(&USBSerialStream, "node address: %i %i\n\r",temp,temp1);
														nrf_config();
													
													}													
										
											}								
									
									}
							}						
						}  // else if		
		/* ******************************************************** */					
						//	Save Node type //
					
						
						else if(strcmp(token,"cfg_node_type")== 0) 	{
							// this token get the node number 0-5	
							if ((token = strtok(NULL,delimiter)))	{
								
									temp = atoi(token);
									// check for valid node number
									if (temp >= 0 && temp <= 5)
									{
												// get the node type 0 to 255
											if ((token = strtok(NULL,delimiter)))	{
								
												temp1 = atoi(token);
												
												if (temp1 >= 0 && temp1 <= 255)
													{
										
														Nodes[temp].NodeType = temp1;	
														NodeStrucToEeprom();
														fprintf(&USBSerialStream, "node type: %i %i\n\r",temp,temp1);
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
						
						
						
} // end of function
