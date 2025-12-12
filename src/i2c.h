#ifndef I2C_H
#define I2C_H

#include <stdint.h>

void twi_init(void);
void twi_start(void);
void twi_write(uint8_t data);
void twi_stop(void);

#endif
