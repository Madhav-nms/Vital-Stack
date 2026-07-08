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

#define MPU6050_ACCEL_CONFIG           0x1C

#define MPU6050_ACCEL_XOUT_H           0x3B
#define MPU6050_ACCEL_XOUT_L           0x3C
#define MPU6050_ACCEL_YOUT_H           0x3D
#define MPU6050_ACCEL_YOUT_L           0x3E
#define MPU6050_ACCEL_ZOUT_H           0x3F
#define MPU6050_ACCEL_ZOUT_L           0x40

typedef struct {
    int16_t X; 
    int16_t Y; 
    int16_t Z; 

}   MPU6050_ACCEL_DATA;

int mpu6050_init(void);                                             // Initialization
int mpu6050_read_accel(MPU6050_ACCEL_DATA *ACCEL_DATA);             // Read the data 

