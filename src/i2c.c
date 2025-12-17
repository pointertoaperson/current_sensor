#include "i2c.h"
#include <avr/io.h>

// Basic blocking I2C routines for 100kHz
void twi_init(void) {
    // SCL frequency = F_CPU / (16 + 2*TWBR*prescaler)
    // prescaler = 1
    TWSR = 0x00; // prescaler = 1
    TWBR = (uint8_t)(((F_CPU / 100000UL) - 16) / 2);
    TWCR = (1<<TWEN);
}

void twi_start(void) {
    TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
    while (!(TWCR & (1<<TWINT)));
}

void twi_write(uint8_t data) {
    TWDR = data;
    TWCR = (1<<TWINT)|(1<<TWEN);
    while (!(TWCR & (1<<TWINT)));
}

void twi_stop(void) {
    TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWSTO);
    
}
