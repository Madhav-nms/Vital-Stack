#include "lcd.h"
#include "i2c.h"
#include "rom/ets_sys.h" 

#define LCD_ADDR      0x27 // I2C address of the LCD backpack
#define LCD_BACKLIGHT 0x08 // bacpackbit of the LCD backlight 
#define RS_COMMAND    0x00 // RS = 0, so the LCD interprets the byte as a command
#define RS_DATA       0x01 // RS = 1, so the LCD interprets the byte as display data

#define LCD_CLEAR      0x01 // clears display
#define LCD_HOME       0x02 // cursor to home position     
#define LCD_4BIT_MODE  0x28 // sets fn to use 4 data bit for comms    
#define LCD_DISPLAY_ON 0x0C // ON the display    
#define LCD_ENTRY_MODE 0x06 // sets the cursor to move right after inc     

static const uint8_t ROW_ADDR[2] = { 0x00, 0x40 }; // DDRAM address for each LCD row

static void send_to_backpack(uint8_t byte) // sending a byte to LCD backpack
{
    i2c_write_reg(LCD_ADDR, byte, 0x00); 
 
}
 
static void pulse_enable(uint8_t byte) // creating Enable pulse so LCD reads the data 
{
    send_to_backpack(byte | 0x04);    
    ets_delay_us(1);
    send_to_backpack(byte & ~0x04);   
    ets_delay_us(50);                 
}


static void send_nibble(uint8_t nibble, uint8_t rs) // send 4 bits of data (D7-D4)
{
    
    uint8_t byte = (nibble & 0xF0) | LCD_BACKLIGHT | rs;
    pulse_enable(byte);
}

// Sending full 8bit value to the LCD using two 4-bit transfers
static void send_byte(uint8_t byte, uint8_t rs)
{
    send_nibble(byte & 0xF0,        rs);   
    send_nibble((byte << 4) & 0xF0, rs);   
}

// configuring and synchronizing LCD 
void lcd_init(void)
{
    
    ets_delay_us(50000);           
 
    
    send_nibble(0x30, RS_COMMAND);  
    ets_delay_us(4500);
    send_nibble(0x30, RS_COMMAND);  
    ets_delay_us(150);
    send_nibble(0x30, RS_COMMAND);  
    ets_delay_us(150);
 
    send_nibble(0x20, RS_COMMAND);  
    ets_delay_us(150);
 
    
    send_byte(LCD_4BIT_MODE,  RS_COMMAND);   
    send_byte(LCD_DISPLAY_ON, RS_COMMAND);   
    send_byte(LCD_CLEAR,      RS_COMMAND);   
    ets_delay_us(2000);                      
    send_byte(LCD_ENTRY_MODE, RS_COMMAND);   
}

//clear display
void lcd_clear(void)
{
    send_byte(LCD_CLEAR, RS_COMMAND);
    ets_delay_us(2000);   
}
// move cursor
void lcd_set_cursor(uint8_t row, uint8_t col)
{
    uint8_t addr = 0x80 | (ROW_ADDR[row] + col);
    send_byte(addr, RS_COMMAND);
}
// print str
void lcd_print(const char *str)
{
    while (*str) {
        send_byte((uint8_t)(*str), RS_DATA);
        str++;
    }
}
// print number 
void lcd_print_int(int value)
{
    char buf[12];
    int  i = 0;

    if (value < 0) {
        send_byte('-', RS_DATA);
        value = -value;
    }
 
    if (value == 0) {
        send_byte('0', RS_DATA);
        return;
    }
 
    while (value > 0) {
        buf[i++] = '0' + (value % 10);   
        value /= 10;
    }

    for (int j = i - 1; j >= 0; j--) {
        send_byte(buf[j], RS_DATA);
    }
}