#ifndef SPI_COMM_H_
#define SPI_COMM_H_

#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>


#define SS_PIN   PB2
#define MOSI_PIN PB3
#define SCK_PIN  PB5


#define CS_LOW()   (PORTB &= ~(1 << SS_PIN))
#define CS_HIGH()  (PORTB |=  (1 << SS_PIN))


void spi_init(void);
void spi_send_byte(uint8_t data);
void spi_send_uint16(uint16_t data);


void spi_send_current(uint16_t peak, uint16_t rms, uint16_t freq);

#endif 