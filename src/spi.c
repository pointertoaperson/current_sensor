#include "spi.h"

// Initialize SPI as master
 spi_init(void) {
    // Set MOSI, SCK, SS as output
    DDRB |= (1 << MOSI_PIN) | (1 << SCK_PIN) | (1 << SS_PIN);
    // Enable SPI, Master, set clock rate fck/16
    SPCR = (1 << SPE) | (1 << MSTR) | (1 << SPR0);
    CS_HIGH(); // Set SS high
}

// Transmit a single byte
 spi_send_byte(uint8_t data) {
    SPDR = data;               
    while (!(SPSR & (1 << SPIF))); 
}

// Transmit a 16-bit value (high byte first)
 spi_send_uint16(uint16_t data) {
    spi_send_byte((data >> 8) & 0xFF); // MSB
    spi_send_byte(data & 0xFF);        // LSB
}

void spi_send_current(uint16_t peak, uint16_t rms, uint16_t freq) {
    CS_LOW();
    _delay_us(5);          

    // Header / command
    spi_send_byte(0x02);   // Command
    spi_send_byte(0x00);   

    // Data
    spi_send_uint16(peak);
    spi_send_uint16(rms);
    spi_send_uint16(freq);



    CS_HIGH();
    _delay_us(5);          
}



