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

#include <stdint.h>
#include <stdbool.h>

#include "sl_icm42688p.h"
#include "sl_icm42688p_defs.h"
#include "sl_imu.h"
#include "sl_sleeptimer.h"

/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */
static uint8_t IMU_state = IMU_STATE_DISABLED;
static float sensorsSampleRate = 0;
static uint32_t IMU_isDataReadyQueryCount = 0;
static uint32_t IMU_isDataReadyTrueCount = 0;
/** @endcond */

/***************************************************************************//**
 * Initialize and calibrate the IMU.
 ******************************************************************************/
sl_status_t sl_imu_init(void)
{
    sl_status_t status;
    uint8_t devid;
    float gyroBiasScaled[3];

    IMU_state = IMU_STATE_INITIALIZING;

    /* Initialize ICM42688P driver */
    status = sl_icm42688p_init();
    if (status != SL_STATUS_OK) {
        goto cleanup;
    }

    status = sl_icm42688p_get_device_id(&devid);
    if (status != SL_STATUS_OK) {
        goto cleanup;
    }

    /* Gyro calibration */
    IMU_state = IMU_STATE_CALIBRATING;
    status = sl_icm42688p_calibrate_gyro(gyroBiasScaled);
    if (status != SL_STATUS_OK) {
        goto cleanup;
    }

    IMU_state = IMU_STATE_INITIALIZING;

cleanup:
    if (status != SL_STATUS_OK) {
        sl_icm42688p_deinit();
        IMU_state = IMU_STATE_DISABLED;
    }

    return status;
}

/***************************************************************************//**
 * De-initialize the IMU chip.
 ******************************************************************************/
sl_status_t sl_imu_deinit(void)
{
    sl_status_t status;

    IMU_state = IMU_STATE_DISABLED;
    status = sl_icm42688p_deinit();

    return status;
}

/***************************************************************************//**
 * Return current IMU state.
 ******************************************************************************/
uint8_t sl_imu_get_state(void)
{
    return IMU_state;
}

/***************************************************************************//**
 * Configure IMU sample rate and enable sensors.
 ******************************************************************************/
void sl_imu_configure(float sampleRate)
{
    uint32_t itStatus;

    IMU_state = IMU_STATE_INITIALIZING;

    /* Enable accelerometer and gyroscope */
    sl_icm42688p_enable_sensor(true, true, false);

    /* Set sample rate */
    sensorsSampleRate = sl_icm42688p_set_sample_rate(sampleRate);

    /* Set full-scale ranges */
    sl_icm42688p_set_full_scale_accel(ICM42688P_ACCEL_CONFIG0_FS_2G);
    sl_icm42688p_set_full_scale_gyro(ICM42688P_GYRO_CONFIG0_FS_250DPS);

    /* Set bandwidth / ODR */
    sl_icm42688p_accel_set_bandwidth(ICM42688P_ODR_CODE_1KHZ);   // 1 kHz accel BW
    sl_icm42688p_gyro_set_bandwidth(ICM42688P_GYRO_ODR_200HZ);   // 200 Hz gyro BW

    sl_sleeptimer_delay_millisecond(50);

    /* Enable raw data ready interrupt */
    sl_icm42688p_enable_interrupt(true);

    /* Clear interrupts */
    sl_icm42688p_read_interrupt_status(&itStatus);

    IMU_state = IMU_STATE_READY;
}

/***************************************************************************//**
 * Retrieve raw acceleration data from the IMU.
 ******************************************************************************/
void sl_imu_get_acceleration(float avec[3])
{
    if (IMU_state != IMU_STATE_READY) {
        avec[0] = avec[1] = avec[2] = 0;
        return;
    }

    sl_icm42688p_accel_read_data(avec);
}

/***************************************************************************//**
 * Retrieve raw gyroscope data from the IMU.
 ******************************************************************************/
void sl_imu_get_gyro(float gvec[3])
{
    if (IMU_state != IMU_STATE_READY) {
        gvec[0] = gvec[1] = gvec[2] = 0;
        return;
    }

    sl_icm42688p_gyro_read_data(gvec);
}

/***************************************************************************//**
 * Perform gyroscope calibration to cancel bias.
 ******************************************************************************/
sl_status_t sl_imu_calibrate_gyro(void)
{
    sl_status_t status;

    /* Disable interrupts during calibration */
    sl_icm42688p_enable_interrupt(false);
    sl_imu_deinit();
    status = sl_imu_init();
    sl_imu_configure(sensorsSampleRate);

    return status;
}

/***************************************************************************//**
 * Check if new accel/gyro data is available.
 ******************************************************************************/
bool sl_imu_is_data_ready(void)
{
    bool ready;

    if (IMU_state != IMU_STATE_READY) {
        return false;
    }

    ready = sl_icm42688p_is_data_ready();
    IMU_isDataReadyQueryCount++;

    if (ready) {
        IMU_isDataReadyTrueCount++;
    }

    return ready;
}
