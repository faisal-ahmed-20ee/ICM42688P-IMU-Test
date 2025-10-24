/***************************************************************************//**
 * @file
 * @brief Register and bit definitions for the TDK InvenSense ICM42688P sensor
 *******************************************************************************
 * SPDX-License-Identifier: Zlib
 *
 * Copyright 2025 Silicon Laboratories Inc.
 ******************************************************************************/

#ifndef SL_ICM42688P_DEFS_H
#define SL_ICM42688P_DEFS_H

#ifdef __cplusplus
extern "C" {
#endif

/* -- Device identification -- */
#define ICM42688P_REG_WHO_AM_I             0x75U
#define ICM42688P_DEVICE_ID                0x47U

/* -- Bank select (write bank in REG_BANK_SEL 0x76) -- */
#define ICM42688P_REG_BANK_SEL             0x76U
#define ICM42688P_BANK_0                   0x00U
#define ICM42688P_BANK_1                   0x01U
#define ICM42688P_BANK_2                   0x02U
#define ICM42688P_BANK_4                   0x04U

/* --- Bank 0 registers (common) --- */
#define ICM42688P_REG_DEVICE_CONFIG        0x11U  /* 0x11 (17) DEVICE_CONFIG */
#define ICM42688P_REG_DRIVE_CONFIG         0x13U  /* 0x13 (19) DRIVE_CONFIG */
#define ICM42688P_REG_INT_CONFIG           0x14U  /* 0x14 (20) INT_CONFIG */
#define ICM42688P_REG_FIFO_CONFIG          0x16U  /* 0x16 (22) FIFO_CONFIG (FIFO_MODE) */
#define ICM42688P_REG_TEMP_DATA1           0x1DU
#define ICM42688P_REG_TEMP_DATA0           0x1EU
#define ICM42688P_REG_ACCEL_DATA_X1        0x1FU
#define ICM42688P_REG_ACCEL_DATA_X0        0x20U
#define ICM42688P_REG_ACCEL_DATA_Y1        0x21U
#define ICM42688P_REG_ACCEL_DATA_Y0        0x22U
#define ICM42688P_REG_ACCEL_DATA_Z1        0x23U
#define ICM42688P_REG_ACCEL_DATA_Z0        0x24U
#define ICM42688P_REG_GYRO_DATA_X1         0x25U
#define ICM42688P_REG_GYRO_DATA_X0         0x26U
#define ICM42688P_REG_GYRO_DATA_Y1         0x27U
#define ICM42688P_REG_GYRO_DATA_Y0         0x28U
#define ICM42688P_REG_GYRO_DATA_Z1         0x29U
#define ICM42688P_REG_GYRO_DATA_Z0         0x2AU
#define ICM42688P_REG_INT_STATUS0          0x2DU
#define ICM42688P_REG_INT_STATUS1          0x2EU
#define ICM42688P_REG_FIFO_COUNTH          0x2EU
#define ICM42688P_REG_FIFO_COUNTL          0x2FU
#define ICM42688P_REG_FIFO_DATA            0x30U

/* Additional Bank0 registers at higher offsets */
#define ICM42688P_REG_SIGNAL_PATH_RESET    0x4BU  /* SIGNAL_PATH_RESET */
#define ICM42688P_REG_INTF_CONFIG0         0x4CU  /* INTF_CONFIG0 */
#define ICM42688P_REG_INTF_CONFIG1         0x4DU  /* INTF_CONFIG1 */
#define ICM42688P_REG_PWR_MGMT0            0x4EU  /* PWR_MGMT0 */

/* --- Bank 1 registers --- */
#define ICM42688P_REG_GYRO_CONFIG0         0x4FU  /* GYRO_CONFIG0: address 0x4F (79) */
#define ICM42688P_REG_ACCEL_CONFIG0        0x50U  /* ACCEL_CONFIG0: address 0x50 (80) */
#define ICM42688P_REG_GYRO_CONFIG1         0x51U
#define ICM42688P_REG_GYRO_ACCEL_CONFIG0   0x52U
#define ICM42688P_REG_ACCEL_CONFIG1        0x53U

/* FIFO config registers (Bank 0 / UI registers) */
#define ICM42688P_REG_FIFO_CONFIG1         0x5FU
#define ICM42688P_REG_FIFO_CONFIG2         0x60U
#define ICM42688P_REG_FIFO_CONFIG3         0x61U
#define ICM42688P_REG_FSYNC_CONFIG         0x62U

/* SELF_TEST_CONFIG (Bank 0) */
#define ICM42688P_REG_SELF_TEST_CONFIG     0x70U  /* SELF_TEST_CONFIG address 0x70 (112) per datasheet */
#define ICM42688P_SELF_TEST_ACCEL_ST_POWER (1U << 6) /* ACCEL_ST_POWER (bit6) */
#define ICM42688P_SELF_TEST_EN_AZ_ST       (1U << 5)
#define ICM42688P_SELF_TEST_EN_AY_ST       (1U << 4)
#define ICM42688P_SELF_TEST_EN_AX_ST       (1U << 3)
#define ICM42688P_SELF_TEST_EN_GZ_ST       (1U << 2)
#define ICM42688P_SELF_TEST_EN_GY_ST       (1U << 1)
#define ICM42688P_SELF_TEST_EN_GX_ST       (1U << 0)

/* ------------------------------------------------------------------------- */
/* GYRO_CONFIG0 fields (address 0x4F)                                         */
/* Bits 7:5 = GYRO_FS_SEL (see datasheet mapping)                            */
/* Bits 3:0 = GYRO_ODR (ODR codes)                                            */
/* ------------------------------------------------------------------------- */
#define ICM42688P_GYRO_CONFIG0_SHIFT_FS_SEL      5U
#define ICM42688P_GYRO_CONFIG0_MASK_FS_SEL       (0x07U << ICM42688P_GYRO_CONFIG0_SHIFT_FS_SEL)

/* GYRO full-scale selection (datasheet mapping: 000=±2000 ... 111=±15.625) */
#define ICM42688P_GYRO_CONFIG0_FS_2000DPS        (0x00U << ICM42688P_GYRO_CONFIG0_SHIFT_FS_SEL)
#define ICM42688P_GYRO_CONFIG0_FS_1000DPS        (0x01U << ICM42688P_GYRO_CONFIG0_SHIFT_FS_SEL)
#define ICM42688P_GYRO_CONFIG0_FS_500DPS         (0x02U << ICM42688P_GYRO_CONFIG0_SHIFT_FS_SEL)
#define ICM42688P_GYRO_CONFIG0_FS_250DPS         (0x03U << ICM42688P_GYRO_CONFIG0_SHIFT_FS_SEL)
#define ICM42688P_GYRO_CONFIG0_FS_125DPS         (0x04U << ICM42688P_GYRO_CONFIG0_SHIFT_FS_SEL)
#define ICM42688P_GYRO_CONFIG0_FS_62_5DPS        (0x05U << ICM42688P_GYRO_CONFIG0_SHIFT_FS_SEL)
#define ICM42688P_GYRO_CONFIG0_FS_31_25DPS       (0x06U << ICM42688P_GYRO_CONFIG0_SHIFT_FS_SEL)
#define ICM42688P_GYRO_CONFIG0_FS_15_625DPS      (0x07U << ICM42688P_GYRO_CONFIG0_SHIFT_FS_SEL)

/* GYRO_ODR codes (bits 3:0) common ODR codes (example) */
#define ICM42688P_GYRO_ODR_RESERVED              0x00U
#define ICM42688P_GYRO_ODR_32KHZ                 0x01U
#define ICM42688P_GYRO_ODR_16KHZ                 0x02U
#define ICM42688P_GYRO_ODR_8KHZ                  0x03U
#define ICM42688P_GYRO_ODR_4KHZ                  0x04U
#define ICM42688P_GYRO_ODR_2KHZ                  0x05U
#define ICM42688P_GYRO_ODR_1KHZ                  0x06U /* default */
#define ICM42688P_GYRO_ODR_200HZ                 0x07U
/* ... other codes available in datasheet ... */

/* ------------------------------------------------------------------------- */
/* ACCEL_CONFIG0 fields (address 0x50)                                       */
/* Bits 7:5 = ACCEL_FS_SEL                                                     */
/* Bits 3:0 = ACCEL_ODR                                                       */
/* ------------------------------------------------------------------------- */
#define ICM42688P_ACCEL_CONFIG0_SHIFT_FS_SEL     5U
#define ICM42688P_ACCEL_CONFIG0_MASK_FS_SEL      (0x07U << ICM42688P_ACCEL_CONFIG0_SHIFT_FS_SEL)

/* ACCEL full-scale selection (datasheet mapping: 000=±16g default, 001=±8g, 010=±4g, 011=±2g) */
#define ICM42688P_ACCEL_CONFIG0_FS_16G           (0x00U << ICM42688P_ACCEL_CONFIG0_SHIFT_FS_SEL)
#define ICM42688P_ACCEL_CONFIG0_FS_8G            (0x01U << ICM42688P_ACCEL_CONFIG0_SHIFT_FS_SEL)
#define ICM42688P_ACCEL_CONFIG0_FS_4G            (0x02U << ICM42688P_ACCEL_CONFIG0_SHIFT_FS_SEL)
#define ICM42688P_ACCEL_CONFIG0_FS_2G            (0x03U << ICM42688P_ACCEL_CONFIG0_SHIFT_FS_SEL)

/* --- ODR codes (examples used by UI) --- */
/* Accelerometer: convert raw (signed 16-bit) -> g */
#define ICM42688P_ACCEL_SCALE_2G           (2.0f / 32768.0f)      /* ACCEL_FS_SEL = 3 -> 16,384 LSB/g -> 1/16384 = 2/32768 */
#define ICM42688P_ACCEL_SCALE_4G           (4.0f / 32768.0f)
#define ICM42688P_ACCEL_SCALE_8G           (8.0f / 32768.0f)
#define ICM42688P_ACCEL_SCALE_16G          (16.0f / 32768.0f)

/* Gyroscope: convert raw (signed 16-bit) -> degrees/sec */
#define ICM42688P_GYRO_SCALE_2000DPS      (2000.0f / 32768.0f)
#define ICM42688P_GYRO_SCALE_1000DPS      (1000.0f / 32768.0f)
#define ICM42688P_GYRO_SCALE_500DPS       (500.0f / 32768.0f)
#define ICM42688P_GYRO_SCALE_250DPS       (250.0f / 32768.0f)
#define ICM42688P_GYRO_SCALE_125DPS       (125.0f / 32768.0f)
#define ICM42688P_GYRO_SCALE_62_5DPS      (62.5f  / 32768.0f)
#define ICM42688P_GYRO_SCALE_31_25DPS     (31.25f / 32768.0f)
#define ICM42688P_GYRO_SCALE_15_625DPS    (15.625f/ 32768.0f)

/* --- Temperature conversion --- */
#define ICM42688P_TEMP_SENSITIVITY         132.48f
#define ICM42688P_TEMP_OFFSET              25.0f

/* --- Misc / FIFO --- */
#define ICM42688P_REG_FIFO_CONFIG1          0x5FU
#define ICM42688P_REG_FIFO_CONFIG2          0x60U
#define ICM42688P_REG_FIFO_CONFIG3          0x61U
#define ICM42688P_FIFO_CONFIG1_FIFO_MODE_MASK 0x07U

/* ------------------------------------------------------------------------- */
/* PWR_MGMT0 register (0x4E) bit definitions                                  */
/* ------------------------------------------------------------------------- */
/* Accelerometer modes (bits 1:0) */
#define ICM42688P_PWR_MGMT0_ACCEL_MODE_OFF       (0x00U)
#define ICM42688P_PWR_MGMT0_ACCEL_MODE_LOWPOWER  (0x02U)
#define ICM42688P_PWR_MGMT0_ACCEL_MODE_LOWNOISE  (0x03U)

/* Gyroscope modes (bits 3:2) */
#define ICM42688P_PWR_MGMT0_GYRO_MODE_OFF        (0x00U)
#define ICM42688P_PWR_MGMT0_GYRO_MODE_STANDBY    (0x04U)
#define ICM42688P_PWR_MGMT0_GYRO_MODE_LOWNOISE   (0x0CU)

/* Temperature sensor disable (bit 5) */
#define ICM42688P_PWR_MGMT0_TEMP_DIS             (1U << 5)

#define ICM42688P_INTF_CONFIG0_I2C_DISABLE   (0x03U)  // bit 1 disables I3C
#define ICM42688P_DEVICE_CONFIG_RESET   0x01
#define ICM42688P_INT_STATUS0_DATA_RDY   (1U << 0)
#define ICM42688P_ACCEL_ODR_MASK 0x0FU
#define ICM42688P_GYRO_ODR_MASK 0x0FU
#define ICM42688P_ODR_CODE_1KHZ 0x06U  // already defined for accel
#define ICM42688P_ODR_CODE_200HZ 0x07U


#define ICM42688P_REG_INT_ENABLE           0x53U   // INT_ENABLE register (example address; replace with datasheet value)
#define ICM42688P_INT1_POLARITY            (1U << 0)  // Bit 0: INT1 polarity (0=active-low, 1=active-high)
#define ICM42688P_INT1_DRIVE_CIRCUIT       (1U << 1)  // Bit 1: INT1 drive circuit (0=open-drain, 1=push-pull)
#define ICM42688P_INT1_MODE                 (1U << 2)  // Bit 2: INT1 mode (0=pulsed, 1=latched)
#define ICM42688P_REG_INT_SOURCE0                 0x65U
#define ICM42688P_INT_SOURCE0_UI_DRDY_INT1_EN     (1 << 3)

#ifdef __cplusplus
}
#endif

#endif // SL_ICM42688P_DEFS_H
