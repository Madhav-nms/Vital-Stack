#include "i2c.h"
#include "driver/gpio.h"
#include "rom/ets_sys.h"

/* I2C conditions :
    - SCL HIGH + SDA falls  → START condition
    - SCL HIGH + SDA rises  → STOP condition
    - Data on SDA is only allowed to change while SCL is LOW
    - After every 8 bits sent, we release SDA and read the ACK bit 
*/

#define SDA_PIN 21 
#define SCL_PIN 22

#define I2C_delay 5 

static inline void sda_high(void) { gpio_set_level(SDA_PIN, 1); }
static inline void sda_low(void)  { gpio_set_level(SDA_PIN, 0); }
static inline void scl_high(void) { gpio_set_level(SCL_PIN, 1); }
static inline void scl_low(void)  { gpio_set_level(SCL_PIN, 0); }
static inline int  sda_read(void) { return gpio_get_level(SDA_PIN); }
 
static inline void delay(void) { ets_delay_us(I2C_delay); }

//Start Cond.

static void start_condition(void)
{
    sda_high();
    scl_high();
    delay();
    sda_low();   
    delay();
    scl_low();  
    delay();
}

// Stop Cond.

static void stop_condition(void)
{
    sda_low();
    delay();
    scl_high();
    delay();
    sda_high(); 
    delay();
}

static int write_byte(uint8_t byte)
{
    // Send 8 bits, most significant bit first 
    for (int i = 7; i >= 0; i--) {
        if (byte & (1 << i)) {
            sda_high();
        } else {
            sda_low();
        }
        delay();
        scl_high();   // slave reads SDA on the rising edge 
        delay();
        scl_low();
        delay();
    }
 
    // Release SDA so the slave can drive it for ACK
    sda_high();
    delay();
    scl_high();
    delay();
 
    // ACK = slave pulls SDA LOW. NACK = SDA stays HIGH 
    int nack = sda_read();
 
    scl_low();
    delay();
 
    return nack ? I2C_ERR : I2C_OK;
}

static uint8_t read_byte(int send_ack)
{
    uint8_t byte = 0;
 
    sda_high();   // release SDA so slave can drive it 
 
    for (int i = 7; i >= 0; i--) {
        delay();
        scl_high();
        delay();
        if (sda_read()) {
            byte |= (1 << i);
        }
        scl_low();
    }
 
    // Send ACK or NACK 
    if (send_ack) {
        sda_low();    // ACK 
    } else {
        sda_high();   // NACK — tells slave we are done 
    }
    delay();
    scl_high();
    delay();
    scl_low();
    delay();
 
    return byte;
}

void i2c_init(void)
{
    gpio_config_t cfg = {
        .pin_bit_mask = (1ULL << SDA_PIN) | (1ULL << SCL_PIN),
        .mode         = GPIO_MODE_OUTPUT_OD,   // open-drain 
        .pull_up_en   = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type    = GPIO_INTR_DISABLE,
    };
    gpio_config(&cfg);
 
    // Idle state: both lines HIGH 
    sda_high();
    scl_high();
}
 
int i2c_write_reg(uint8_t addr, uint8_t reg, uint8_t data)
{
    start_condition();
 
    // Send device address with write bit (bit 0 = 0) 
    if (write_byte((addr << 1) | 0x00) != I2C_OK) goto err;
 
    // Send register address 
    if (write_byte(reg) != I2C_OK) goto err;
 
    // Send data byte 
    if (write_byte(data) != I2C_OK) goto err;
 
    stop_condition();
    return I2C_OK;
 
err:
    stop_condition();
    return I2C_ERR;
}
 
int i2c_read_reg(uint8_t addr, uint8_t reg, uint8_t *buf, uint8_t len)
{
    start_condition();
 
    // Phase 1 — tell the device which register we want 
    if (write_byte((addr << 1) | 0x00) != I2C_OK) goto err;
    if (write_byte(reg) != I2C_OK) goto err;
 
    // Phase 2 — repeated START, then read 
    start_condition();
    if (write_byte((addr << 1) | 0x01) != I2C_OK) goto err;  // read bit 
 
    for (uint8_t i = 0; i < len; i++) {
        // ACK all bytes except the last one 
        buf[i] = read_byte(i < len - 1);
    }
 
    stop_condition();
    return I2C_OK;
 
err:
    stop_condition();
    return I2C_ERR;
}