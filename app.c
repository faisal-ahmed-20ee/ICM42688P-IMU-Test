/***************************************************************************//**
 * @file
 * @brief Top level application functions for IMU raw data reading
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************/

#include "app.h"
#include "sl_imu.h"
#include "sl_sleeptimer.h"
#include <stdio.h>  // for printf if UART is retargeted

// Use float arrays to match sl_imu_get_acceleration/gyro
static float accel[3];
static float gyro[3];

void app_init(void)
{
    // Initialize IMU
    if (sl_imu_init() != SL_STATUS_OK) {
        // Initialization failed, handle error (e.g., blink LED)
        printf("IMU initialization failed!\r\n");
        return;
    }

    // Configure IMU sample rate (1 kHz)
    sl_imu_configure(1000.0f);
    printf("IMU initialized and configured.\r\n");
}

void app_process_action(void)
{
    // Check if new data is ready
    if (sl_imu_is_data_ready()) {
        // Read raw acceleration and gyro data
        sl_imu_get_acceleration(accel);
        sl_imu_get_gyro(gyro);

        // Print acceleration data (X, Y, Z)
        printf("Accel: X=%.2f Y=%.2f Z=%.2f\r\n",
               accel[0], accel[1], accel[2]);

        // Print gyroscope data (X, Y, Z)
        printf("Gyro:  X=%.2f Y=%.2f Z=%.2f\r\n",
               gyro[0], gyro[1], gyro[2]);

        // Small delay to avoid flooding UART
        sl_sleeptimer_delay_millisecond(100);
    }
}
