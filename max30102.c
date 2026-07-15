#include "max30102.h"
#include "i2c.h"
#include "rom/ets_sys.h"

int max30102_init(void){
    ets_delay_us(100000);  // 100ms powerup delay
    uint8_t part_id;
    //0x57, Read Register, store it in part_id, 1byte
    if(i2c_read_reg(MAX30102_ADDR, REG_PART_ID, &part_id, 1) != I2C_OK) return -1;
    if(part_id != MAX30102_PART_ID) return -1;

    // soft reset (refer page 18)
    if(i2c_write_reg(MAX30102_ADDR, REG_MODE_CONFIG, 0x40) != I2C_OK) return -1;
    ets_delay_us(10000);

    // 4 samples average, rollover ON, 15 empty slots (refer page 17)
    if(i2c_write_reg(MAX30102_ADDR, REG_FIFO_CONFIG, 0x5F) != I2C_OK) return -1;

    //SpO2 configuration (ADC: 4096nA, pulse width: 411us, sample rate: 100sps) (page 18)
    if(i2c_write_reg(MAX30102_ADDR, REG_SPO2_CONFIG, 0x27) != I2C_OK) return -1;

    // LED config (7.2mA)
    if(i2c_write_reg(MAX30102_ADDR, REG_LED1_PA, 0x24) != I2C_OK) return -1; // RED LED
    if(i2c_write_reg(MAX30102_ADDR, REG_LED2_PA, 0x24) != I2C_OK) return -1; // IR LED

    // Enable Interrupt (PPG_RDY page 12)
    if(i2c_write_reg(MAX30102_ADDR, REG_INT_ENABLE1, 0x40) != I2C_OK) return -1;

    // Clear FIFO (to start with empty FIFO)
    if(i2c_write_reg(MAX30102_ADDR, REG_FIFO_WR_PTR, 0x00) != I2C_OK) return -1;
    if(i2c_write_reg(MAX30102_ADDR, REG_FIFO_RD_PTR, 0x00) != I2C_OK) return -1;
    if(i2c_write_reg(MAX30102_ADDR, REG_OVF_COUNTER, 0x00) != I2C_OK) return -1;

    // Set Mode Config to SpO2
    if(i2c_write_reg(MAX30102_ADDR, REG_MODE_CONFIG, 0x03) != I2C_OK) return -1;

    return 0;
}

int max30102_read_fifo(max30102_sample_t *sample){
    // FIFO is 18 bits, so 3 bytes. (RED and IR 3 each, so 6 bytes)
    uint8_t buf[6];
    if(i2c_read_reg(MAX30102_ADDR, REG_FIFO_DATA, buf, 6) != I2C_OK) return -1;

    // dereferencing the pointer AND accessing a member of the struct it points to
    // red is 18bits, the first 6 bits are reserved so AND buf[0] with 0x03 to get the 2 digits and then left shift by 16, followed by the next byte
    sample->red = ((uint32_t)(buf[0] & 0x03) << 16) | ((uint32_t)buf[1] << 8) | buf[2];
    sample->ir  = ((uint32_t)(buf[3] & 0x03) << 16) | ((uint32_t)buf[4] << 8) | buf[5];

    return 0;
}

void max30102_clear_interrupt(void) {
    uint8_t clear;
    i2c_read_reg(MAX30102_ADDR, REG_INT_STATUS1, &clear, 1);
}