#pragma once 
#include <stdint.h>

#define MAX30102_ADDR 0x57 // Refer: page 29 table 17 

// Registers (Refer page 10)
#define REG_INT_STATUS1     0x00
#define REG_INT_ENABLE1     0x02
#define REG_FIFO_WR_PTR     0x04
#define REG_OVF_COUNTER     0x05
#define REG_FIFO_RD_PTR     0x06
#define REG_FIFO_DATA       0x07
#define REG_FIFO_CONFIG     0x08
#define REG_MODE_CONFIG     0x09
#define REG_SPO2_CONFIG     0x0A
#define REG_LED1_PA         0x0C
#define REG_LED2_PA         0x0D
#define REG_PART_ID         0xFF

#define MAX30102_PART_ID    0x15 

// ADC is 18 bit
typedef struct {
    uint32_t red;
    uint32_t ir; 
}max30102_sample_t;

int max30102_init(void);                               // Initialization
int max30102_read_fifo(max30102_sample_t *sample);     // Read the data 
void max30102_clear_interrupt(void);                   // clear the INT pin 