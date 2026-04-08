
# Vital Signs Data Logger

Bare-metal vital signs logger built on ESP32 — custom I2C, SPI, and UART drivers written from scratch without HAL abstraction.

## Overview 

Embedded system for real time acquisition, display and logging of physiological and motion data using an ESP32. The system integrates multiple sensors and peripherals, scheduled using FreeRTOS. 

## Features

- Real Time heart rate and SpO2 acquisition (MAX30102)
- Motion sensing via IMU (MPU6050)
- Live data display on I2C LCD
- SD card logging with timestamps
- UART streaming to PC for monitoring/ debugging
- Mutli task scheduling using FreeRTOS

## System Architecture 

### Core Controller 
- ESP32 (FreeRTOS based task scheduling)

### Sensors 
- MAX30102 (Heart rate + SpO2) - I2C
- MPU6050 (Accelerometer + Gyroscope) - I2C

### Output / Storage

- 1602 LCD - I2C (live display)
- SD Card Module - SPI (data logging)
- CP2102 UART - PC interface (data streaming)

### Debugging 

Logic Analyzer used for signal verification (I2C, SPI, UART)

