#pragma once
#include <stdbool.h>
#include "mpu6050.h"
 
// MPU6050-only test mode: the task polls the sensor at a fixed rate (no INT pin needed)
#define SAMPLE_PERIOD_MS       10          // 100 Hz polling
#define PRINT_EVERY_N          10          // print 10 lines/s so UART doesn't slow the loop
 
#define MOTION_THRESHOLD_G     0.3f        // |magnitude - 1g| above this = motion
#define ROTATION_THRESHOLD_DPS 30.0f       // any axis spinning faster than this = motion
 
extern volatile bool g_motion_detected;
extern MPU6050_DATA g_imu_data;
 
int vital_task_start(void);