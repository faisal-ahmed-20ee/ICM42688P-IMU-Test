/***************************************************************************//**
 * @file
 * @brief Driver for the TDK InvenSense ICM42688P 6-axis motion sensor
 *******************************************************************************
 * SPDX-License-Identifier: Zlib
 *
 * Copyright 2025 Silicon Laboratories Inc.
 ******************************************************************************/

#ifndef SL_ICM42688P_H
#define SL_ICM42688P_H

#include <stdint.h>
#include <stdbool.h>
#include "sl_status.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "sl_icm42688p_defs.h"
#include "sl_icm42688p_config.h"

/* Public API */
sl_status_t sl_icm42688p_spi_init(void);
sl_status_t sl_icm42688p_init(void);
sl_status_t sl_icm42688p_deinit(void);
sl_status_t sl_icm42688p_reset(void);

sl_status_t sl_icm42688p_read_register(uint8_t reg, uint8_t *data, uint16_t len);
sl_status_t sl_icm42688p_write_register(uint8_t reg, uint8_t data);
sl_status_t sl_icm42688p_masked_write(uint8_t addr, uint8_t data, uint8_t mask);

sl_status_t sl_icm42688p_set_bank(uint8_t bank);

sl_status_t sl_icm42688p_set_full_scale_accel(uint8_t fs_code);
sl_status_t sl_icm42688p_set_full_scale_gyro(uint8_t fs_code);
float       sl_icm42688p_set_sample_rate(float sample_rate);

sl_status_t sl_icm42688p_enable_sensor(bool accel, bool gyro, bool temp);
sl_status_t sl_icm42688p_enable_interrupt(bool data_ready_enable);

sl_status_t sl_icm42688p_accel_read_data(float accel[3]);
sl_status_t sl_icm42688p_gyro_read_data(float gyro[3]);
sl_status_t sl_icm42688p_read_temperature(float *temperature);

sl_status_t sl_icm42688p_get_device_id(uint8_t *dev_id);
bool        sl_icm42688p_is_data_ready(void);

sl_status_t sl_icm42688p_calibrate_gyro(float bias[3]);
sl_status_t sl_icm42688p_accel_set_bandwidth(uint8_t odr_code);
sl_status_t sl_icm42688p_gyro_set_bandwidth(uint8_t odr_code);
sl_status_t sl_icm42688p_read_interrupt_status(uint32_t *status);

sl_status_t sl_icm42688p_accel_get_resolution(float *accel_res);
sl_status_t sl_icm42688p_gyro_get_resolution(float *gyro_res);

#ifdef __cplusplus
}
#endif

#endif // SL_ICM42688P_H
