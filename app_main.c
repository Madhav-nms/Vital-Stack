#include "i2c.h"
#include "vital_task.h"
#include "mpu6050.h"
#include <stdio.h>

// MPU6050-only test build. MAX30102 init is skipped; its driver files are untouched.

void app_main(void){
    i2c_init();
    if(mpu6050_init()!= 0) {
        printf("Failed to initialize MPU6050\n");
        return;
    }
    if(vital_task_start()!=0) {
        printf("Failed to start the vital_task\n");
        return;
    }
}