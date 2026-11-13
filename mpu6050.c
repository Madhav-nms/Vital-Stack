#include "mpu6050.h"
#include "i2c.h"
#include "rom/ets_sys.h"
#include <stdio.h>
#include <stdint.h>

int mpu6050_init(void) {
    uint8_t chip_id = 0;
    // 0x75 read register and store the value in chip_id of 1 byte
    if(i2c_read_reg(MPU6050_ADDR, MPU6050_WHO_AM_I, &chip_id, 1 ) != I2C_OK) return -1;
    if (chip_id != MPU6050_WHO_AM_I_VAL)  {
        printf("WHO_AM_I = 0x%02X\n", chip_id);
        return -1;
    }

    // Device reset (PWR_MGMT_1 bit7 = DEVICE_RESET) so we always start from a known state
    if(i2c_write_reg(MPU6050_ADDR, MPU6050_PWR_MGMT_1, 0x80) != I2C_OK) return -1;
    ets_delay_us(100000);   // 100 ms for reset to complete

    // Wake the device up (page 41 - 42)
    // SLEEP=0, CYCLE=0, TEMP_DIS=1, CLKSEL=001 (PLL with X-gyro reference — recommended once the gyro is running)
    if(i2c_write_reg(MPU6050_ADDR, MPU6050_PWR_MGMT_1, 0x09) != I2C_OK ) return -1;
    // LP_WAKE_CTRL = 00, STBY_XA YA ZA = 000, STBY_XG YG ZG = 000 -> all 6 axes ON
    if(i2c_write_reg(MPU6050_ADDR, MPU6050_PWR_MGMT_2, 0x00) != I2C_OK) return -1;
    ets_delay_us(50000);    // let the gyro/PLL settle

    //Sample Rate down to exactly 1kHz to match the accelerometer's actual output rate (page 11)
    //The chip boots with SMPLRT_DIV = 0x00 setting the gyro output rate to 8KHz because of which accelerometer sample gets repeated multiple times
    if(i2c_write_reg(MPU6050_ADDR, MPU6050_SMPLRT_DIV, 0x07) != I2C_OK) return -1;

    // Config register (page 13)
    // EXT_SYNC_SET = 000, DLPF_CFG = 000 (A sudden sharp jostle is high frequency content and it shouldnt be attenuated)
    if(i2c_write_reg(MPU6050_ADDR, MPU6050_CONFIG, 0x00) != I2C_OK) return -1;

    // Gyroscope Configuration (page 14)
    // XG_ST YG_ST ZG_ST = 000, FS_SEL = 01 (+-500 dps) -> covers fast wrist rotation without saturating
    if(i2c_write_reg(MPU6050_ADDR, MPU6050_GYRO_CONFIG, 0x08) != I2C_OK) return -1;

    // Aceelerometer Configuration (page 15)
    // Reason for 8g: detecting motion artifacts that corrupt PPG/SpO2 reading from MAX30102
    // The kind of motion that corrupts a PPG reading is sharp and sudden : someone moving their wrist quickly, bumping the sensor, etc.
    // 8g because it is the spot where it won't saturate on real motion and still has enough resolution to detect it.
    if(i2c_write_reg(MPU6050_ADDR, MPU6050_ACCEL_CONFIG, 0x10) != I2C_OK ) return -1; // XA_ST YA_ST ZA_ST = 000, AFS_SEL = 10 (8g)

    return 0;

}

int mpu6050_read_accel(MPU6050_ACCEL_DATA *ACCEL_DATA) {

    uint8_t byte[6];

    // Burst read 6 bytes starting at ACCEL_XOUT_H (0x3B), auto incrementing till 0x40
    if( i2c_read_reg (MPU6050_ADDR, MPU6050_ACCEL_XOUT_H, byte, 6) != I2C_OK) return -1;

    // Reconstruct 16-bit signed values: H byte shifted to MSB, L byte in lower 8 bits
    ACCEL_DATA -> X = (int16_t)((byte[0] << 8) | byte[1]);
    ACCEL_DATA -> Y = (int16_t)((byte[2] << 8) | byte[3]);
    ACCEL_DATA -> Z = (int16_t)((byte[4] << 8) | byte[5]);

    return 0;
}

int mpu6050_read_all(MPU6050_DATA *data) {

    uint8_t byte[14];

    // Burst read 14 bytes 0x3B..0x48 in ONE transaction so accel and gyro come from the same sample
    // [0..5] accel X/Y/Z, [6..7] temperature (disabled, skipped), [8..13] gyro X/Y/Z
    if (i2c_read_reg(MPU6050_ADDR, MPU6050_ACCEL_XOUT_H, byte, 14) != I2C_OK) return -1;

    data->ax = (int16_t)((byte[0]  << 8) | byte[1]);
    data->ay = (int16_t)((byte[2]  << 8) | byte[3]);
    data->az = (int16_t)((byte[4]  << 8) | byte[5]);
    data->gx = (int16_t)((byte[8]  << 8) | byte[9]);
    data->gy = (int16_t)((byte[10] << 8) | byte[11]);
    data->gz = (int16_t)((byte[12] << 8) | byte[13]);

    return 0;
}
