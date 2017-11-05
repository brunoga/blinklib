/*
 * Access the service port 
 *
 * The service port has a high speed (500Kbps) bi-directional serial connection plus an Aux pin that can be used
 * as either digitalIO or an analog in. 
 *
 * Mostly useful for debugging, but maybe other stuff to? :)
 * 
 */ 

#include "hardware.h"

#include "utils.h"

#include "sp.h"


// Read the analog voltage on service port pin A
// Returns 0-255 for voltage between 0 and Vcc
// Handy to connect a potentiometer here and use to tune params
// like rightness or speed

uint8_t sp_aux_analogRead(void) {
    
	ADMUX =
	_BV(REFS0)   |                  // Reference AVcc voltage
	_BV( ADLAR ) |                  // Left adjust result so only one 8 bit read of the high register needed
	_BV( MUX2 )  | _BV( MUX1 )      // Select ADC6
	;
	
	ADCSRA =
	_BV( ADEN )  |                  // Enable ADC
	_BV( ADSC )                     // Start a conversion
	;
	
	
	while (TBI(ADCSRA,ADSC)) ;       // Wait for conversion to complete
	
	return( ADCH );
	
}


// Initialize the serial on the service port. 

void sp_serial_init(void) {
    
    //Initialize the AUX pin as digitalOut    
    //SBI( SP_AUX_DDR , SP_AUX_PIN );
    
    // Initialize SP serial port for 500K baud, n-8-1
    // This feels like it belongs in hardware.c, maybe in an inline fucntion?
        
    SBI( SP_SERIAL_CTRL_REG , U2X0);        // 2X speed
    SBI( UCSR0B , TXEN0 );                  // Enable transmitter (disables DEBUGA)
    SBI( UCSR0B , RXEN0);                   // Enable receiver    (disables DEBUGB)
    
    #if F_CPU!=4000000  
        #error Serial port calculation in debug.cpp must be updated if not 4Mhz CPU clock.
    #endif
    
    UBRR0 = 0;                  // 500Kbd. This is as fast as we can go at 4Mhz, and happens to be 0% error and supported by the Arduino serial monitor. 
                                // See datasheet table 25-7. 
        
}    

// Send a byte out the serial port. DebugSerialInit() must be called first. Blocks unitl buffer free if TX already in progress.

void sp_serial_tx(uint8_t b) {    
        
    while (!TBI(SP_SERIAL_CTRL_REG,UDRE0));         // Wait for buffer to be clear so we don't overwrite in progress
    
    SP_SERIAL_DATA_REG=b;                           // Send new byte
    
}

// Is there a char ready to read?

uint8_t sp_serial_rx_ready(void) {
    
    return TBI( SP_SERIAL_CTRL_REG , SP_SERIAL_READY_BIT );
    
}    

// Read byte from service port serial. Blocks if nothing received yet. 

uint8_t sp_serial_rx(void) {
    
    while ( !TBI( SP_SERIAL_CTRL_REG , SP_SERIAL_READY_BIT ) );
    
    return( SP_SERIAL_DATA_REG );
    
}

