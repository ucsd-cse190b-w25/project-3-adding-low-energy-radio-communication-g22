#ifndef I2C_H
#define I2C_H

#include <stdint.h>

void i2c_init();
uint8_t i2c_transaction(uint8_t address, uint8_t dir, uint8_t* data, uint8_t len);

#endif