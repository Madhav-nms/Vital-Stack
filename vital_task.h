#pragma once 
#include <stdbool.h>
#include "mpu6050.h"
#include "max30102.h"
             
#define MAX30102_INT_PIN       4           // Interrupt pin GPIO of MAX30102 when wired 
#define MOTION_THRESHOLD_G    1.5f         // acceleration above this (in g's) flags PPG sample as corrupted data due to motion
extern bool g_motion_detected;               
extern max30102_sample_t g_ppg_sample;
extern MPU6050_ACCEL_DATA g_accel_data;

int vital_task_start();
