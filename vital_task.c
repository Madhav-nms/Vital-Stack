#include "freertos/FreeRTOS.h"     // defines core types
#include "freertos/task.h"         // TaskHandle_t, xTaskCreate(), vTaskDelayUntil()
#include "vital_task.h"
#include "mpu6050.h"
#include <math.h>
#include <stdio.h>
#include <stdint.h>

volatile bool g_motion_detected = false;
MPU6050_DATA g_imu_data;

static TaskHandle_t s_sensor_task;

// FreeRTOS Task — polls the MPU6050 every SAMPLE_PERIOD_MS
static void sensor_task(void *pvParameters) {
    TickType_t last_wake = xTaskGetTickCount();
    uint32_t count = 0;

    while (1) {
        // Fixed-rate loop: wakes exactly every SAMPLE_PERIOD_MS regardless of how long the body takes
        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(SAMPLE_PERIOD_MS));

        if (mpu6050_read_all(&g_imu_data) != 0) {
            printf("Error reading mpu6050\n");
            continue;
        }

        // Convert raw counts to physical units
        float ax = MPU6050_COUNTS_TO_G(g_imu_data.ax);
        float ay = MPU6050_COUNTS_TO_G(g_imu_data.ay);
        float az = MPU6050_COUNTS_TO_G(g_imu_data.az);
        float gx = MPU6050_COUNTS_TO_DPS(g_imu_data.gx);
        float gy = MPU6050_COUNTS_TO_DPS(g_imu_data.gy);
        float gz = MPU6050_COUNTS_TO_DPS(g_imu_data.gz);

        // At rest |a| = 1g (gravity), so motion = deviation from 1g, in either direction
        float magnitude = sqrtf((ax * ax) + (ay * ay) + (az * az));
        bool linear_motion = fabsf(magnitude - 1.0f) > MOTION_THRESHOLD_G;

        // Gyro catches pure rotation that barely changes |a|
        bool rotation = fabsf(gx) > ROTATION_THRESHOLD_DPS ||
                        fabsf(gy) > ROTATION_THRESHOLD_DPS ||
                        fabsf(gz) > ROTATION_THRESHOLD_DPS;

        g_motion_detected = linear_motion || rotation;

        if (++count % PRINT_EVERY_N == 0) {
            printf("A[g] %6.2f %6.2f %6.2f |a|=%5.2f  G[dps] %7.1f %7.1f %7.1f  %s\n",
                   ax, ay, az, magnitude, gx, gy, gz,
                   g_motion_detected ? "MOTION" : "still");
        }
    }
}

// Called once by app_main() to create the FreeRTOS task
int vital_task_start(void) {
    // Priority 3, stack 4096 BYTES (ESP-IDF uses bytes, not words); printf with floats needs the room
    BaseType_t task_created = xTaskCreate(sensor_task, "sensor_task", 4096, NULL, 3, &s_sensor_task);
    if (task_created != pdPASS) {
        printf("Task creation failed\n");
        return -1;
    }
    return 0;
}