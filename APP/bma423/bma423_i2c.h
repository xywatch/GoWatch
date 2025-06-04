#ifndef __BMA423_I2C_H
#define __BMA423_I2C_H

#include <stdint.h>

// I2C read/write functions for BMA423
uint16_t _readRegister(uint8_t address, uint8_t reg, uint8_t *data, uint16_t len);
uint16_t _writeRegister(uint8_t address, uint8_t reg, uint8_t *data, uint16_t len);

#endif /* __BMA423_I2C_H */ 