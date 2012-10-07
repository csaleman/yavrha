/*
			Yavrha Project
			Carlos Silva, 2012
		
             LUFA Library
     Copyright (C) Dean Camera, 2011.

  dean [at] fourwalledcubicle [dot] com
           www.lufa-lib.org
*/

#include "yavrha.h"
#include "nRF24L01.h"

// Define Radio nRF24L01 Radio Setting
uint8_t EEMEM CH;
uint8_t EEMEM eeNRF_ADDRESS[4];	
tNodesData EEMEM eeNodes[MAXSNODES];
tNodesData Nodes[MAXSNODES];
	
// Pin definitions for chip select chip enable and SPI MEGA32U4
#define CSN  PB0
#define CE	 PB4
#define CLK  PB1
#define MOSI PB2
#define MISO PB3 
#define DDR_ISP DDRB
#define PORT_ISP PORTB

// Definitions for selecting and enabling the radio
#define CSN_HIGH      PORTB |=  (1<<CSN);    // SPI bus INVERSE HIGH to disable
#define CSN_LOW       PORTB &= ~(1<<CSN);    // SPI bus enable LOW to enable
#define CE_HIGH       PORTB |=  (1<<CE);     // Enable radio to receive or to send (10mS)
#define CE_LOW        PORTB &= ~(1<<CE);     // Disable "standby" receive


// Function Prototypes
void spi_init(void);
void nrf_config(void);
void nrf_read_register(uint8_t, uint8_t *, uint8_t);
uint8_t nrf_command(uint8_t);
void nrf_config_register(uint8_t, uint8_t);
uint8_t nrf_read_payload(void);
void spi_transfer_sync (uint8_t *, uint8_t *, uint8_t);
void spi_transmit_sync (uint8_t *, uint8_t);
uint8_t spi_fast_shift (uint8_t);
void nrf_TXnodeCfg(uint8_t, uint8_t *);


// Since individual bits cannot be modified, all CONFIG settings must be set up at start up.
// EN_CRC enable CRC, CRCO set up 2 bytes CRC. 
// This is a primary RX, PRIM_RX must be set.
#define RX_POWERUP	  nrf_config_register(CONFIG,(1<<PWR_UP)|(1<<EN_CRC) | (1<<CRCO) | (1<<PRIM_RX) );
#define TX_POWERUP	  nrf_config_register(CONFIG,(1<<PWR_UP)|(1<<EN_CRC) | (1<<CRCO));
#define POWERDOWN	  nrf_config_register(CONFIG, (1<<EN_CRC) | (1<<CRCO));


// ************************************************************************************************ //
 // ISP Functions. Not to be used by any function other than nrf_*
 // For ISP for MEGA32U4
 // Initialize pins for spi communication
	void spi_init(void)
	
	{
		DDR_ISP |= (1<<CSN) | (1<<CLK) | (1<<MOSI) | (1<<CE);
		// Pull-UP for MISO
		PORT_ISP |= (1<<MISO); 
		
		// Since CSN is CS "NON", Disable Slave
		PORT_ISP |= (1<<CSN);
		
		// Set the ISP Control Register
		
		SPCR = (1<<SPE) | (1<<MSTR) | (1<<SPR1) | (1<<SPR0);
		
	}		

void spi_transfer_sync (uint8_t * dataout, uint8_t * datain, uint8_t len)
// Shift full array through target device
// This send the LSByte first, as required by NRF24L01
{
       uint8_t i;      
       for (i = len; i > 0; i--) {
             // Load data in SPDR
			 SPDR = dataout[i-1];
			 
				while(!(SPSR & (1<<SPIF)))
				 {
					;  // Do nothing just wait for transfer
				}
             datain[i-1] = SPDR;
       }
}

void spi_transmit_sync (uint8_t * dataout, uint8_t len)
// Shift full array to target device without receiving any byte
// This send the LSByte first, as required by NRF24L01
{
       uint8_t i;      
       for (i = len; i > 0; i--) {
             SPDR = dataout[i-1];
			   
				while(!(SPSR & (1<<SPIF)))
				 {
					 ;  // Do nothing just wait for transfer
				}
             
       }
}

uint8_t spi_fast_shift (uint8_t data)
// Clocks only one byte to target device and returns the received one
{
    SPDR = data;
     
	while(!(SPSR & (1<<SPIF)));		// Wait for transfer
	
    return SPDR;
}


// ***********************************      END of SPI Functions     ***************************************** //

// ***********************************      NRF24L01 Basic Functions ***************************************** //                      

uint8_t nrf_command(uint8_t reg)
// Send command to radio without argument; Used ONLY for FLUSH_TX, FLUSH_RX, NOP. Return status register.
// Do not use with W_TX_PAYLOAD OR R_RX_PAYLOAD
{
	uint8_t status_reg;
    
	CSN_LOW  // ISP Slave on
    status_reg = spi_fast_shift(reg);
    CSN_HIGH // ISP Slave off

	return status_reg;
}


void nrf_config_register(uint8_t reg, uint8_t value)
// Clocks only one byte into the given NRF24L01+ register
{
    CSN_LOW  // ISP Slave on
    spi_fast_shift(W_REGISTER | (REGISTER_MASK & reg));
    spi_fast_shift(value);
    CSN_HIGH // ISP Slave off
}

void nrf_read_register(uint8_t reg, uint8_t * value, uint8_t len)
// Reads an array of bytes from the given start position in the NRF24L01+ registers.
{
    CSN_LOW     // ISP Slave on
    spi_fast_shift(R_REGISTER | (REGISTER_MASK & reg));
    spi_transfer_sync(value,value,len);
    CSN_HIGH    // ISP Slave off
}

void nrf_write_register(uint8_t reg, uint8_t * value, uint8_t len) 
// Writes an array of bytes into into NRF24L01+ registers.
{
	CSN_LOW
    spi_fast_shift(W_REGISTER | (REGISTER_MASK & reg));
    spi_transmit_sync(value,len);
    CSN_HIGH
}

uint8_t nrf_send_completed()
/* Return 1 for TX_DS flag "successful", 2 for MAX_RT (unsuccessful),
 otherwise return 0. This is useful to wait until package has been send or fail.
*/
{
	uint8_t status;
	uint8_t return_status;
	
	CSN_LOW
	status = spi_fast_shift(NOP);		// Execute a NOP operation to get status register.
	CSN_HIGH
		
	if ( status & (1<<TX_DS))
	{
		return_status =  1;
	} 
	
	else if (status & (1<<MAX_RT))
	{
		return_status = 2;
	}
	else
	{
		return_status = 0;
	}
	
	return return_status;
	
}


/*	Read received data from NRF radio				*/
/*	return the pipe # from where data was received. */
uint8_t nrf_read_payload(void) 

{
    
	uint8_t status;		// to store the status register from NRF
	uint8_t	pipe_number;		// to store pipe number
	
	// Disable receiving while reading payload
	CE_LOW	
    
    CSN_LOW                    // Pull down chip select
    status = spi_fast_shift( R_RX_PAYLOAD ); // Send Read Payload Command
    spi_transfer_sync(buffer,buffer,PAYLOAD_WIDTH);   // Read payload
    CSN_HIGH                    // Pull up chip select
	
	nrf_config_register(STATUS,(1<<RX_DR)); // clear status register flag, write 1 to clear bit.
	pipe_number = (status >> 1) & 0b00000111;
	
	// Enable receiving again
	CE_HIGH

	return pipe_number;
}
// ***********************************      END NRF24L01 Basic Functions     ***************************************** //

// Configure the NRF as a receiver.

void nrf_config(void)

{
	// Temp variable to hold EEPROM NRF_ADDR
	uint8_t ram_NRF_ADDR[5];
	uint8_t enable_rxaddr = 0;
	
	
	// Deactivate radio before changing configuration
	 CE_LOW
	
	// Copy Node Information from EEPROM to RAM
	NodeStructToRam();
	
	// Configure radio channel
	nrf_config_register(RF_CH,eeprom_read_byte(&CH));
	
	// Configure Speed, Comment for default 2Mbps
	// 0 for 1Mbps and (1<<RF_DR_LOW) for 250kbps
	// nrf_config_register(RF_SETUP,(1<<RF_DR_LOW));
	
	// Configure NRF RX ADDRESS
	// LOAD eeNRF_ADDRESS from EEPROM into ramNRF_ADDR
	eeprom_read_block((void *)&ram_NRF_ADDR, (const void *)&eeNRF_ADDRESS,5);
	
	
/*******************************    To configure PIPES         ************************/
	/* This use 4 MSB Bytes from NRF_ADDR and add the P0 addr byte to create the 
		5 bytes address required by NRF; the same with P1. Then load the Payload witdth and 
		set bit in enable_rxaddr for each enabled node. Else disable the node. */
	
	if (Nodes[0].NodeEnable == 1) {							// Test if the node is enable
		ram_NRF_ADDR[4] = Nodes[0].NodeAddr;				// Create 5 Bytes address using P0 LSB
		nrf_write_register(RX_ADDR_P0, ram_NRF_ADDR, 5);	// Load address in NRF
		nrf_config_register(RX_PW_P0,PAYLOAD_WIDTH);		// Configure Payload Width for this Pipe
		enable_rxaddr |= (1<<ERX_P0);						// Change bit to 00000001
	}		
	else enable_rxaddr &= ~(1<<ERX_P0);						// Else, node is disable 00000000
	
	if (Nodes[1].NodeEnable == 1) {
		ram_NRF_ADDR[4] = Nodes[1].NodeAddr;
		nrf_write_register(RX_ADDR_P1, ram_NRF_ADDR, 5);
		nrf_config_register(RX_PW_P1,PAYLOAD_WIDTH);
		enable_rxaddr |= (1<<ERX_P1); 
	}
		
	else enable_rxaddr &= ~(1<<ERX_P1); 
	
	if (Nodes[2].NodeEnable == 1) {
		nrf_write_register(RX_ADDR_P1, ram_NRF_ADDR, 5);	// P1 address Byte must be set to allow P2 to P5 work
		nrf_config_register(RX_ADDR_P2,Nodes[2].NodeAddr);
		nrf_config_register(RX_PW_P2,PAYLOAD_WIDTH);
		enable_rxaddr |= (1<<ERX_P2); 
	}
	else enable_rxaddr &= ~(1<<ERX_P2); 
	
	if (Nodes[3].NodeEnable == 1) {
		nrf_write_register(RX_ADDR_P1, ram_NRF_ADDR, 5);
		nrf_config_register(RX_ADDR_P3,Nodes[3].NodeAddr); 
		nrf_config_register(RX_PW_P3,PAYLOAD_WIDTH);	
		enable_rxaddr |= (1<<ERX_P3);
	}
	else enable_rxaddr &= ~(1<<ERX_P3); 
	
	if (Nodes[4].NodeEnable == 1) {
		nrf_write_register(RX_ADDR_P1, ram_NRF_ADDR, 5);
		nrf_config_register(RX_ADDR_P4,Nodes[4].NodeAddr);
		nrf_config_register(RX_PW_P4,PAYLOAD_WIDTH);
		enable_rxaddr |= (1<<ERX_P4);
	}
	
	else enable_rxaddr &= ~(1<<ERX_P4); 
	
	if (Nodes[5].NodeEnable == 1) {
		nrf_write_register(RX_ADDR_P1, ram_NRF_ADDR, 5);
		nrf_config_register(RX_ADDR_P5,Nodes[5].NodeAddr);
		nrf_config_register(RX_PW_P5,PAYLOAD_WIDTH);
		enable_rxaddr |= (1<<ERX_P5);
	}
	else enable_rxaddr &= ~(1<<ERX_P5); 
		
	nrf_config_register(EN_RXADDR,enable_rxaddr);
/*******************************    End configure PIPES         ************************/	

	// clear old data from RX FIFO
	nrf_command(FLUSH_RX);
	
	// Activate radio
	RX_POWERUP
	
	// Activate PRX Mode
	CE_HIGH
	
	// Enable Radio Interrupt
	EIMSK |= (1<<INT0);
}

// ************* Configure radio as TX to send configuration others node **********************//

void nrf_TXnodeCfg(uint8_t tx_ch, uint8_t * tx_addr)
// Configure radio parameters for pTX
{
	
	// Disable Radio RX Interrupt
	EIMSK &= ~(1<<INT0);
	
	CE_LOW		// Put radio is in standby;
	
	
	// Configure default radio channel
	nrf_config_register(RF_CH,tx_ch);
	
	// Configure NRF default RX ADDRESS, required for Shockburst
	nrf_write_register(RX_ADDR_P0, tx_addr, 5);
	
	// Configure NRF TX ADDRESS
	nrf_write_register(TX_ADDR, tx_addr, 5);
	
	// enable P0, required for auto ack to work 
	nrf_config_register(EN_RXADDR,1);
	
	// Flush any old data in TX FIFO.
	nrf_command(FLUSH_TX);		

	TX_POWERUP
}

//************************* Send Data to remote Node *************************//
/* This funtion send the data to a RX node, it most be call right after nrf_TXnodeCfg()
// only argument is a pointer with the data array, return 1 if data was received by node, 
2 MAX_RT "no received", or 0 either yet.
*/
uint8_t send_node_data(uint8_t * node_data)
{
	uint8_t send_result;	// to store send result
	uint8_t i;
	
	CSN_LOW                    // Pull down chip select
    spi_fast_shift( W_TX_PAYLOAD ); // Send Write Payload Command
    spi_transfer_sync(node_data,node_data,PAYLOAD_WIDTH);   // write data
    CSN_HIGH                    // Pull up chip select
	   
    CE_HIGH                     // Start transmission
	_delay_us(20);			// Short Delay to make sure CE is High for at least 10us
	CE_LOW
										// Wait until data is sent or MAX_RT flag
	// This function return 1 if data was received by node, 2 MAX_RT "no received", or 0 either yet.
	// The i loop with the constant is to avoid infinite loops. 
	do {
			send_result = nrf_send_completed();
			i++;
		} while ( send_result == 0 && i<50);

	nrf_config_register(STATUS,(1<<TX_DS)|(1<<MAX_RT));	// Reset NRF Interrupt write 1 to clear bit.
	
	
	return send_result;
	
	
}



// ************************* Send Node Configuration to remote Node *************************//
//  This function is just to be used after setting this radio to default configuration 
// Configuration packet: {ADDR0,ADDR1,ADDR2,ADDR3,ADDR4,CH }
uint8_t send_node_cfg(uint8_t node) {
	
	uint8_t ram_NRF_ADDR[5];
	uint8_t send_result;	// to store send result
	uint8_t i;
	
	// LOAD eeNRF_ADDRESS from EEPROM into ramNRF_ADDR
	 eeprom_read_block((void *)&ram_NRF_ADDR, (const void *)&eeNRF_ADDRESS,5);
	
		buffer[0] = ram_NRF_ADDR[0];
		buffer[1] = ram_NRF_ADDR[1];
		buffer[2] = ram_NRF_ADDR[2];
		buffer[3] = ram_NRF_ADDR[3]; 
		buffer[4] = Nodes[node].NodeAddr;
		buffer[5] = Nodes[node].Node_Number;
		buffer[6] = eeprom_read_byte(&CH);
	
	nrf_config_register(STATUS,(1<<TX_DS)|(1<<MAX_RT));	// Reset NRF Interrupt write 1 to clear bit.
	
	CSN_LOW                    // Pull down chip select
    spi_fast_shift( W_TX_PAYLOAD ); // Send Read Payload Command
    spi_transfer_sync(buffer,buffer,CFG_PAYLOAD_WIDTH);   // Read payload
    CSN_HIGH                    // Pull up chip select
	   
    CE_HIGH                     // Start transmission
	_delay_us(20);			// Short Delay to make sure CE is High for at least 10us
	CE_LOW
										// Wait until data is sent or MAX_RT flag
	// This function return 1 if data was received by node, 2 MAX_RT "no received", or 0 either yet.
	do {
			send_result = nrf_send_completed();
			i++;
		} while ( send_result == 0 && i<50);

	nrf_config_register(STATUS,(1<<TX_DS)|(1<<MAX_RT));	// Reset NRF Interrupt write 1 to clear bit.
	
	
	return send_result;
}


// functions defined to be used by others sources.
void startRadio(void){

	CE_HIGH	
}	

// functions defined to be used by others sources.
void stopRadio(void){

	CE_LOW	
}	

// Copy EEPROM NODE Structure to RAM

void NodeStructToRam(void){
	
	eeprom_read_block((void *)&Nodes,(const void *)&eeNodes,sizeof(tNodesData)*MAXSNODES);
	
}

// Save Nodes ram struct to EEPROM
void NodeStrucToEeprom(void){
	
	eeprom_update_block((const void *)&Nodes,(void *)&eeNodes,sizeof(tNodesData)*MAXSNODES);
	
}

