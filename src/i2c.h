#pragma once
#include <stdint.h>

/*
 * TWI/I2C - master, polling (no interrupts)
 *
 * TWSR: prescaler bits = 00 -> prescaler 1
 * TWBR: 72 -> SCL = 16 MHz / (16 + 2*72*1) = 100 kHz
 * TWCR: TWEN=1 enables the TWI peripheral
 *
 * SDA = PC4, SCL = PC5
 */

void i2c_init(void);
void i2c_write(uint8_t addr, uint8_t data);
