#include "freertos/FreeRTOS.h"     // defines core types
#include "freertos/task.h"         // TaskHandle_t and xTaskCreate()
#include "freertos/semphr.h"       // SemaphoreHandle_t, xSemaphoreCreateBinary(), xSemaphoreTake(), xSemaphoreGiveFromISR()
#include "driver/gpio.h"           // gpio_config_t, gpio_install_isr_service(), gpio_isr_handler_add()
#include "rom/ets_sys.h"
#include "i2c.h"
#include "vital_task.h"
#include "mpu6050.h"
#include "max30102.h"
#include <math.h>
#include <stdio.h>

bool g_motion_detected = false;
max30102_sample_t g_ppg_sample;
MPU6050_ACCEL_DATA g_accel_data;

static SemaphoreHandle_t s_isr_semaphore;
static TaskHandle_t s_sensor_task;

static void IRAM_ATTR isr_handler(void *arg) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;                          // starts false, set to true if higher priority task woken
    xSemaphoreGiveFromISR(s_isr_semaphore, &xHigherPriorityTaskWoken);      // raises the flag — wakes sensor task
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);                           // immediate context switch if higher priority task woken
}

// FreeRTOS Task
static void sensor_task(void *pvParameters) {
    while(1) {
        BaseType_t result = xSemaphoreTake(s_isr_semaphore, pdMS_TO_TICKS(1000));
        if (result == pdTRUE) {
            // Read PPG data from MAX30102
            if (max30102_read_fifo(&g_ppg_sample) != 0) {
                printf("Error reading max30102\n");
                continue;
            }
            // Read accelerometer data from MPU6050 at same moment
            if (mpu6050_read_accel(&g_accel_data) != 0) {
                printf("Error reading mpu6050\n");
                continue;
            }
            // Convert raw counts to g's
            float ax = MPU6050_COUNTS_TO_G(g_accel_data.X);
            float ay = MPU6050_COUNTS_TO_G(g_accel_data.Y);
            float az = MPU6050_COUNTS_TO_G(g_accel_data.Z);

            // Calculate total acceleration magnitude
            float magnitude = sqrtf((ax * ax) + (ay * ay) + (az * az));

            // Qualify PPG sample based on motion threshold
            if (magnitude > MOTION_THRESHOLD_G) {
                g_motion_detected = true;   // corrupted — motion detected
            } else {
                g_motion_detected = false;  // clean — no significant motion
            }
        } else {
            // Timeout — no interrupt for 1 second, check if sensors are alive
            uint8_t id = 0;
            if (i2c_read_reg(MPU6050_ADDR, MPU6050_WHO_AM_I, &id, 1) != I2C_OK || id != MPU6050_WHO_AM_I_VAL) {
                printf("MPU6050 not responding\n");
            }
            if (i2c_read_reg(MAX30102_ADDR, REG_PART_ID, &id, 1) != I2C_OK || id != MAX30102_PART_ID) {
                printf("MAX30102 not responding\n");
            }
        }
    }
}

// Called once by app_main() to set up semaphore, GPIO interrupt, and FreeRTOS task
int vital_task_start(void) {
    // Create binary semaphore — ISR gives it, task takes it
    s_isr_semaphore = xSemaphoreCreateBinary();
    if (s_isr_semaphore == NULL) {
        printf("Failed to create semaphore\n");
        return -1;
    }

    // Configure GPIO pin as input, pull-up enabled, falling edge interrupt
    // MAX30102 INT is active low — pulls low when FIFO threshold reached
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << MAX30102_INT_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_NEGEDGE
    };
    if (gpio_config(&io_conf) != ESP_OK) {
        printf("GPIO config failed\n");
        return -1;
    }

    // Install ISR service and attach handler to MAX30102 INT pin
    if (gpio_install_isr_service(0) != ESP_OK) {
        printf("ISR service install failed\n");
        return -1;
    }
    if (gpio_isr_handler_add(MAX30102_INT_PIN, isr_handler, NULL) != ESP_OK) {
        printf("ISR handler add failed\n");
        return -1;
    }

    // Create FreeRTOS task — priority 3, stack 4096 words
    BaseType_t task_created = xTaskCreate(sensor_task, "sensor_task", 4096, NULL, 3, &s_sensor_task);
    if (task_created != pdPASS) {
        printf("Task creation failed\n");
        return -1;
    }

    return 0;
}