#include "i2c.h"
#include <avr/io.h>

void i2c_init(void) {
    TWSR = 0;           // prescaler 1
    TWBR = 72;          // SCL = 16 MHz / (16 + 2*72) = 100 kHz
    TWCR = (1 << TWEN); // enable TWI
}

static void twi_start(void) {
    TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
    while (!(TWCR & (1 << TWINT)));
}

static void twi_stop(void) {
    TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);
    while (TWCR & (1 << TWSTO)); // wait for STOP condition to finish
}

static void twi_byte(uint8_t d) {
    TWDR = d;
    TWCR = (1 << TWINT) | (1 << TWEN);
    while (!(TWCR & (1 << TWINT)));
}

void i2c_write(uint8_t addr, uint8_t data) {
    twi_start();
    twi_byte(addr << 1); // 7-bit address + write bit (0)
    twi_byte(data);
    twi_stop();
}
