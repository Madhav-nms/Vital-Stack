#include "i2c.h"
#include "vital_task.h"
#include "mpu6050.h"
#include "max30102.h"
#include <stdio.h>



void app_main(){
    i2c_init();
    if(max30102_init()!=0) {
        printf("Failed to initialize MAX30102\n");
        return;
    }
    if(mpu6050_init()!= 0) {
        printf("Failed to initialize MPU6050\n");
        return;
    }
    if(vital_task_start()!=0) {
        printf("Failed to start the vital_task\n");
        return;
    }
}