#pragma once
#include <stdint.h>

// Register Map (page 16)

#define MPU6050_WHO_AM_I               0x75
#define MPU6050_WHO_AM_I_VAL           0x68
#define MPU6050_ADDR                   0x68

#define MPU6050_PWR_MGMT_1             0x6B
#define MPU6050_PWR_MGMT_2             0x6C

#define MPU6050_SMPLRT_DIV             0x19

#define MPU6050_CONFIG                 0x1A

#define MPU6050_GYRO_CONFIG            0x1B
#define MPU6050_ACCEL_CONFIG           0x1C

// Accel: AFS_SEL = 2 (+-8g)     -> 4096 LSB/g   (page 29)
#define MPU6050_ACCEL_SENSITIVITY     4096.0f
#define MPU6050_COUNTS_TO_G(counts)   ((float)(counts) / MPU6050_ACCEL_SENSITIVITY)

// Gyro: FS_SEL = 1 (+-500 dps)  -> 65.5 LSB/dps (page 31)
#define MPU6050_GYRO_SENSITIVITY      65.5f
#define MPU6050_COUNTS_TO_DPS(counts) ((float)(counts) / MPU6050_GYRO_SENSITIVITY)

#define MPU6050_ACCEL_XOUT_H           0x3B
#define MPU6050_ACCEL_XOUT_L           0x3C
#define MPU6050_ACCEL_YOUT_H           0x3D
#define MPU6050_ACCEL_YOUT_L           0x3E
#define MPU6050_ACCEL_ZOUT_H           0x3F
#define MPU6050_ACCEL_ZOUT_L           0x40
#define MPU6050_TEMP_OUT_H             0x41
#define MPU6050_TEMP_OUT_L             0x42
#define MPU6050_GYRO_XOUT_H            0x43
#define MPU6050_GYRO_XOUT_L            0x44
#define MPU6050_GYRO_YOUT_H            0x45
#define MPU6050_GYRO_YOUT_L            0x46
#define MPU6050_GYRO_ZOUT_H            0x47
#define MPU6050_GYRO_ZOUT_L            0x48

typedef struct {
    int16_t X;
    int16_t Y;
    int16_t Z;

}   MPU6050_ACCEL_DATA;

// Raw 6-axis sample (counts)
typedef struct {
    int16_t ax, ay, az;     // accelerometer
    int16_t gx, gy, gz;     // gyroscope
} MPU6050_DATA;

int mpu6050_init(void);                                             // Initialization
int mpu6050_read_accel(MPU6050_ACCEL_DATA *ACCEL_DATA);             // Read accel only
int mpu6050_read_all(MPU6050_DATA *data);                           // Read accel + gyro (6-axis)