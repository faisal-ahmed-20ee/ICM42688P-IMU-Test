/***************************************************************************//**
 * @file
 * @brief Inertial Measurement Unit driver for ICM42688P (raw accel & gyro only)
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 ******************************************************************************/

#ifndef SL_IMU_H
#define SL_IMU_H

#include <stdint.h>
#include <stdbool.h>
#include "sl_status.h"

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************//**
* @name State Definitions
* @{
******************************************************************************/
#define IMU_STATE_DISABLED         0x00
#define IMU_STATE_READY            0x01
#define IMU_STATE_INITIALIZING     0x02
#define IMU_STATE_CALIBRATING      0x03
/**@}*/

/***************************************************************************//**
 * @brief Initialize and calibrate the IMU chip.
 ******************************************************************************/ 
sl_status_t sl_imu_init(void);

/***************************************************************************//**
 * @brief De-initialize the IMU chip.
 ******************************************************************************/ 
sl_status_t sl_imu_deinit(void);

/***************************************************************************//**
 * @brief Return IMU state.
 ******************************************************************************/ 
uint8_t sl_imu_get_state(void);

/***************************************************************************//**
 * @brief Configure IMU sample rate and sensor settings.
 ******************************************************************************/ 
void sl_imu_configure(float sampleRate);

/***************************************************************************//**
 * @brief Retrieve raw acceleration data from the IMU.
 ******************************************************************************/ 
void sl_imu_get_acceleration(float avec[3]);

/***************************************************************************//**
 * @brief Retrieve raw gyroscope data from the IMU.
 ******************************************************************************/ 
void sl_imu_get_gyro(float gvec[3]);

/***************************************************************************//**
 * @brief Perform gyroscope calibration to cancel bias.
 ******************************************************************************/ 
sl_status_t sl_imu_calibrate_gyro(void);

/***************************************************************************//**
 * @brief Check if new accel/gyro data is available.
 ******************************************************************************/ 
bool sl_imu_is_data_ready(void);

#ifdef __cplusplus
}
#endif

#endif // SL_IMU_H
