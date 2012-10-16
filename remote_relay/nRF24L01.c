/*
 * nRF24L01.c
 *
 * Created: 3/23/2012 6:48:23 PM
 *  Author: John
 */ 
#include "yavrha_remote_relay.h"

uint8_t EEMEM CH;
uint8_t EEMEM eeNODE_NUMBER;
uint8_t EEMEM eeNRF_ADDRESS[5];
uint8_t NODE_NUMBER;
// Function Prototypes

void spi_init();
void spi_transfer_sync (uint8_t *, uint8_t *, uint8_t);
void spi_transmit_sync (uint8_t *, uint8_t);
uint8_t spi_fast_shift (uint8_t);
void nrf_read_register(uint8_t, uint8_t *, uint8_t);
uint8_t nrf_command(uint8_t);
void nrf_config_register(uint8_t, uint8_t);
void nrf_send(uint8_t *);
void remote_nrf_config(void);
void nrf_read_payload(void); 
void remote_cfg_toEEPROM(uint8_t *);
void nrf_tx_config();
void nrf_rx_config();
void nrf_read_cfg_payload(void);

// ************************************************************************************************ //
	//ISP Functions
	// For ISP for ATMEGA168PA
	// Initialize pins for spi communication
	void spi_init(void)
	
	{
		DDR_ISP |= (1<<CSN) | (1<<CLK) | (1<<MOSI) | (1<<CE);
		
		// Enable MISO Pull-UP
		PORT_ISP |= (1<<MISO);
		
		// Do not enable Pull-UP for IRQ, this increase power consumption for 80uA.
		// NRF24L01 already has PULL-UP.
		//PORT_IRQ |= (1<<IRQ);
		
		// Since CSN is CS "NON", Disable Slave
		PORT_ISP |= (1<<CSN);
		
		/* Set the ISP Control Register
			Enable SPI, Master, and set speed to fclock/16 
		*/
		 SPCR = (1<<SPE) | (1<<MSTR) | (1<<SPR0);

	}

void spi_transfer_sync (uint8_t *dataout, uint8_t *datain, uint8_t len)
// Shift full array through target device
// This send the LSByte first, as required by NRF24L01
{
	   uint8_t i, temp;      
	   
	   for (i = len; i > 0; i--) {
             // Load data in SPDR
			 SPDR = dataout[i-1];
			 
				while(!(SPSR & (1<<SPIF)))
				 {
					;  // Do nothing just wait for transfer
				}
             datain[i-1] = SPDR;
			 
			 temp = datain[i-1];
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
     
	while(!(SPSR & (1<<SPIF)))
		{
			;  // Do nothing just wait for transfer
		}
	
	return SPDR;
}



// ***********************************      END of SPI Functions     ***************************************** //

// ***********************************      NRF24L01 Basic Functions ***************************************** //                      

uint8_t nrf_command(uint8_t reg)
// Send command to radio without argument; Used for FLUSH_TX, FLUSH_RX, NOP. Return status register.
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
// Return 1 if MAX_RT or TX_DS flag triggers, otherwise return 0.
{
	uint8_t status;
	
	status = nrf_command(NOP);
		
	if ((status & (1<<MAX_RT)) || (status & (1<<TX_DS)))
	{
		return 1;
	} 
	else
	{
		return 0;
	}
	
}

void nrf_send(uint8_t * value) 
// Sends a data package to the default address. Be sure to send the correct
// amount of bytes as configured as payload on the receiver.
{
    CE_LOW

    TX_POWERUP                     // Power up NRF
    
    nrf_config_register(STATUS,(1<<TX_DS)|(1<<MAX_RT)); // clear status register, write 1 to clear bit.
	
	
	CSN_LOW                    // Pull down chip select
    spi_fast_shift( FLUSH_TX );     // Write cmd to flush tx fifo
    CSN_HIGH                    // Pull up chip select
    
    CSN_LOW                    // Pull down chip select
    spi_fast_shift( W_TX_PAYLOAD ); // Write cmd to write payload
    spi_transmit_sync(value,PAYLOAD_WIDTH);   // Write payload
    CSN_HIGH                    // Pull up chip select
    
    CE_HIGH                     // Start transmission
	_delay_loop_1(4);			// Short Delay to make sure CE is High for at least 10us
	CE_LOW
										// Wait until data is sent or MAX_RT flag
	while(!(nrf_send_completed()));		// This function return 1 if data was transmitted or after MAX_RT.
										
}
// ***********************************      END NRF24L01 Basic Functions     ***************************************** //


void nrf_tx_config()
// Configure radio parameters for pTX
{
	uint8_t ram_NRF_ADDR[5];
	
	// Make sure radio is in standby
	CE_LOW
	
	// SET RADIO TO TX MODE 
	TX_POWERUP
	
	// Configure NRF RX ADDRESS
	// LOAD eeNRF_ADDRESS from EEPROM into ramNRF_ADDR
	eeprom_read_block((void *)&ram_NRF_ADDR, (const void *)&eeNRF_ADDRESS,5);
	
	// Configure radio channel
	nrf_config_register(RF_CH,eeprom_read_byte(&CH));
	
	// Configure NRF RX ADDRESS
	nrf_write_register(RX_ADDR_P0, ram_NRF_ADDR, 5);
	
	// Configure NRF TX ADDRESS
	nrf_write_register(TX_ADDR, ram_NRF_ADDR, 5);
	
	// This reset the NRF IRQ , RX_DR, TX_DS, MAX_RT
	nrf_config_register(STATUS,(1<<RX_DR)|(1<<TX_DS)|(1<<MAX_RT));
	
}

void nrf_rx_config()
// Configure radio parameters for pRX
{
	uint8_t ram_NRF_ADDR[5];
	
	// Make sure radio is in standby
	CE_LOW
	
	// Set radio to RX Mode
	RX_POWERUP
	
	// Configure NRF RX ADDRESS
	// LOAD eeNRF_ADDRESS from EEPROM into ramNRF_ADDR
	eeprom_read_block((void *)&ram_NRF_ADDR, (const void *)&eeNRF_ADDRESS,5);
	
	// Configure radio channel
	nrf_config_register(RF_CH,eeprom_read_byte(&CH));
	
	// Configure NRF RX ADDRESS
	nrf_write_register(RX_ADDR_P0, ram_NRF_ADDR, 5);
	
	// Configure Payload Width for this Pipe
	nrf_config_register(RX_PW_P0, PAYLOAD_WIDTH);		
	
	// This reset the NRF IRQ , RX_DR, TX_DS, MAX_RT
	nrf_config_register(STATUS,(1<<RX_DR)|(1<<TX_DS)|(1<<MAX_RT));
	
	
	// Flush any data in RX register.
	nrf_command(FLUSH_RX);		
	
	// Activate PRX Mode
	CE_HIGH
	
}


// ***********************************  Functions for Remote Configuration   ****************************************** //

// Configure the NRF as a receiver. In order to get configuration data from master

void remote_nrf_config(void)

{
	// NRF Default Configuration
	uint8_t default_NRF_ADDR[5] = {0xE7,0xE7,0xE7,0xE7,0xE7};
	uint8_t default_NRF_CH = 2;	
				
	
	// Deactivate radio before changing configuration
	 CE_LOW
	 
	 // Set radio to RX Mode
	RX_POWERUP
	
	// This reset the NRF IRQ , RX_DR, TX_DS, MAX_RT
	nrf_config_register(STATUS,(1<<RX_DR)|(1<<TX_DS)|(1<<MAX_RT));
	
	
	// Configure default radio channel
	nrf_config_register(RF_CH,default_NRF_CH);
	
	// Configure P0 to default values 
		nrf_write_register(RX_ADDR_P0, default_NRF_ADDR, 5);	// Load address in NRF
		
		nrf_config_register(RX_PW_P0, CFG_PAYLOAD_WIDTH);		// Configure Payload Width for this Pipe
	
	
	// Activate PRX Mode
	CE_HIGH
	
	nrf_command(FLUSH_RX);		// Flush any data in RX register.
	
	// Enable Radio Interrupt
	EIMSK |= (1<<INT0);
	

}

void nrf_read_payload(void) 
// Sends a data package to the default address. Be sure to send the correct
// amount of bytes as configured as payload on the receiver.
{
	
    CSN_LOW                    // Pull down chip select
    spi_fast_shift( R_RX_PAYLOAD ); // Send Read Payload Command
    spi_transfer_sync(buffer,buffer, PAYLOAD_WIDTH);   // Read payload
    CSN_HIGH                    // Pull up chip select
	
	// clear status register flag IRQ, write 1 to clear bit.
	nrf_config_register(STATUS,(1<<RX_DR) | (1<<TX_DS) | (1<<MAX_RT)); 
	
	DATA0 = buffer[0];
	DATA1 = buffer[1];
	DATA2 = buffer[2];
	DATA3 = buffer[3];
	RECV_MSGID = buffer[4];
	RECV_NODE_NUMBER = buffer[5];
	
}

void nrf_read_cfg_payload(void) 
// Sends a data package to the default address. Be sure to send the correct
// amount of bytes as configured as payload on the receiver.
{
		uint8_t data_input[CFG_PAYLOAD_WIDTH];
		uint8_t trash_out[CFG_PAYLOAD_WIDTH];
	
	
    CSN_LOW                    // Pull down chip select
    spi_fast_shift( R_RX_PAYLOAD ); // Send Read Payload Command
    spi_transfer_sync(trash_out,data_input, CFG_PAYLOAD_WIDTH);   // Read payload
    CSN_HIGH                    // Pull up chip select
	
	// save received data in eeprom
	remote_cfg_toEEPROM(data_input);
	
	// clear status register flag IRQ, write 1 to clear bit.
	nrf_config_register(STATUS,(1<<RX_DR) | (1<<TX_DS) | (1<<MAX_RT)); 
}


// Save the received configuration data in eeprom
void remote_cfg_toEEPROM(uint8_t * data_to_EE){
	

	// store received Configuration CH to EEPROM
	eeprom_update_byte(&CH,data_to_EE[6]);
	
	// store received Configuration Node Number to EEPROM
	eeprom_update_byte(&eeNODE_NUMBER,data_to_EE[5]);
	
	// store received configuration address to eeprom
	eeprom_update_byte(&eeNRF_ADDRESS[4],data_to_EE[4]);
	eeprom_update_byte(&eeNRF_ADDRESS[3],data_to_EE[3]);
	eeprom_update_byte(&eeNRF_ADDRESS[2],data_to_EE[2]);
	eeprom_update_byte(&eeNRF_ADDRESS[1],data_to_EE[1]);
	eeprom_update_byte(&eeNRF_ADDRESS[0],data_to_EE[0]);

}


