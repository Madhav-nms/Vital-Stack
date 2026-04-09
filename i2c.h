#ifndef I2C_H
#define I2C_H 

#include <stdint.h>

#define I2C_OK    0 
#define I2C_ERR   1  // no ack is received from slave 

void i2c_init(void); // configure SDA, SCL pins and call 

int i2c_write_reg(uint8_t addr, uint8_t reg, uint8_t data); /* to write a byte to register 
(7bit I2C device addr, register addr, data ) */

int i2c_read_reg(uint8_t addr, uint8_t reg, uint8_t *buf, uint8_t len); /* read one or more bytes \
(7 bit I2C device addr, register addr, output buffer, number of bytes to read) */

#endif 