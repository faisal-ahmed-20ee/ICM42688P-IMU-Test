/***************************************************************************//**
 * @file
 * @brief Driver for the TDK InvenSense ICM42688P 6-axis motion sensor
 *******************************************************************************
 * SPDX-License-Identifier: Zlib
 *
 * Copyright 2025 Silicon Laboratories Inc.
 ******************************************************************************/

#include "sl_icm42688p.h"
#include "sl_icm42688p_defs.h"
#include "sl_icm42688p_config.h"
#include <stdint.h>
#include <string.h>

#include "sl_status.h"
#include "sl_sleeptimer.h"
#include "sl_gpio.h"
#include "sl_clock_manager.h"
#include "em_device.h"

#if defined(_SILICON_LABS_32B_SERIES_2)
#include "em_eusart.h"
#include "em_gpio.h"
#else
#include "sl_hal_eusart.h"
#include "em_gpio.h"
#endif

/* Local helpers */
static void sl_icm42688p_chip_select_set(bool select);
static void sl_icm42688p_hw_delay_short(void);

/* ----- SPI init ----- */
sl_status_t sl_icm42688p_spi_init(void)
{
  EUSART_TypeDef *eusart = SL_ICM42688P_SPI_EUSART_PERIPHERAL;

#if defined(_SILICON_LABS_32B_SERIES_2)
  EUSART_SpiInit_TypeDef init = EUSART_SPI_MASTER_INIT_DEFAULT_HF;
  EUSART_SpiAdvancedInit_TypeDef advancedInit = EUSART_SPI_ADVANCED_INIT_DEFAULT;

  init.bitRate = 3300000UL;
  init.advancedSettings = &advancedInit;

  advancedInit.autoCsEnable = false;
  advancedInit.msbFirst = true;
#else
  sl_hal_eusart_spi_config_t init = SL_HAL_EUSART_SPI_MASTER_INIT_DEFAULT_HF;
  sl_hal_eusart_spi_advanced_config_t advancedInit = SL_HAL_EUSART_SPI_ADVANCED_INIT_DEFAULT;

  init.advanced_config = &advancedInit;

  advancedInit.auto_cs_enable = false;
  advancedInit.msb_first = true;
#endif

  /* Enable clocks to EUSART */
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_GPIO);
  sl_clock_manager_enable_bus_clock(SL_BUS_CLOCK_EUSART1);  // Directly enable EUSART1

  /* IO Configuration */
  sl_gpio_set_pin_mode(&(sl_gpio_t){SL_ICM42688P_SPI_EUSART_TX_PORT, SL_ICM42688P_SPI_EUSART_TX_PIN}, SL_GPIO_MODE_PUSH_PULL, 0);
  sl_gpio_set_pin_mode(&(sl_gpio_t){SL_ICM42688P_SPI_EUSART_RX_PORT, SL_ICM42688P_SPI_EUSART_RX_PIN}, SL_GPIO_MODE_INPUT, 0);
  sl_gpio_set_pin_mode(&(sl_gpio_t){SL_ICM42688P_SPI_EUSART_SCLK_PORT, SL_ICM42688P_SPI_EUSART_SCLK_PIN}, SL_GPIO_MODE_PUSH_PULL, 0);
  sl_gpio_set_pin_mode(&(sl_gpio_t){SL_ICM42688P_SPI_EUSART_CS_PORT, SL_ICM42688P_SPI_EUSART_CS_PIN}, SL_GPIO_MODE_PUSH_PULL, 1);

  /* Initialize EUSART, in SPI master mode. */
#if defined(_SILICON_LABS_32B_SERIES_2)
  EUSART_SpiInit(eusart, &init);
#else
  sl_hal_eusart_init_spi(eusart, &init);
  sl_hal_eusart_enable(eusart);
  sl_hal_eusart_enable_tx(eusart);
  sl_hal_eusart_enable_rx(eusart);
#endif

  /* Route pins for the chosen EUSART instance */
  GPIO->EUSARTROUTE[SL_ICM42688P_SPI_EUSART_PERIPHERAL_NO].TXROUTE   = (SL_ICM42688P_SPI_EUSART_TX_PORT << _GPIO_EUSART_TXROUTE_PORT_SHIFT) | (SL_ICM42688P_SPI_EUSART_TX_PIN << _GPIO_EUSART_TXROUTE_PIN_SHIFT);
  GPIO->EUSARTROUTE[SL_ICM42688P_SPI_EUSART_PERIPHERAL_NO].RXROUTE   = (SL_ICM42688P_SPI_EUSART_RX_PORT << _GPIO_EUSART_RXROUTE_PORT_SHIFT) | (SL_ICM42688P_SPI_EUSART_RX_PIN << _GPIO_EUSART_RXROUTE_PIN_SHIFT);
  GPIO->EUSARTROUTE[SL_ICM42688P_SPI_EUSART_PERIPHERAL_NO].SCLKROUTE = (SL_ICM42688P_SPI_EUSART_SCLK_PORT << _GPIO_EUSART_SCLKROUTE_PORT_SHIFT) | (SL_ICM42688P_SPI_EUSART_SCLK_PIN << _GPIO_EUSART_SCLKROUTE_PIN_SHIFT);
  GPIO->EUSARTROUTE[SL_ICM42688P_SPI_EUSART_PERIPHERAL_NO].ROUTEEN   = GPIO_EUSART_ROUTEEN_RXPEN | GPIO_EUSART_ROUTEEN_TXPEN | GPIO_EUSART_ROUTEEN_SCLKPEN;

  return SL_STATUS_OK;
}


/* ----- Core init ----- */
sl_status_t sl_icm42688p_init(void)
{
  uint8_t who = 0;

  sl_icm42688p_spi_init();

  /* Soft reset */
  sl_icm42688p_reset();

  /* Wait for reset */
  sl_sleeptimer_delay_millisecond(100);

  /* Read WHO_AM_I */
  sl_icm42688p_read_register(ICM42688P_REG_WHO_AM_I, &who, 1);
  if (who != ICM42688P_DEVICE_ID) {
    return SL_STATUS_INITIALIZATION;
  }

  /* Disable I2C and ensure SPI-only */
  sl_icm42688p_write_register(ICM42688P_REG_INTF_CONFIG0, (uint8_t)ICM42688P_INTF_CONFIG0_I2C_DISABLE);

  /* Power up: enable accel & gyro low-noise mode and enable temperature */
  uint8_t pwr = (uint8_t)(ICM42688P_PWR_MGMT0_ACCEL_MODE_LOWNOISE | ICM42688P_PWR_MGMT0_GYRO_MODE_LOWNOISE);
  pwr &= (uint8_t)(~ICM42688P_PWR_MGMT0_TEMP_DIS);
  sl_icm42688p_write_register(ICM42688P_REG_PWR_MGMT0, pwr);

  /* small startup delay */
  sl_sleeptimer_delay_millisecond(30);

  /* Configure default FS/ODR (Bank 1) */
  sl_icm42688p_set_full_scale_accel((ICM42688P_ACCEL_CONFIG0_FS_16G >> ICM42688P_ACCEL_CONFIG0_SHIFT_FS_SEL));
  sl_icm42688p_set_full_scale_gyro((ICM42688P_GYRO_CONFIG0_FS_2000DPS >> ICM42688P_GYRO_CONFIG0_SHIFT_FS_SEL));

  /* Interrupt pin configuration: PC6 as input + ext-int rising edge */
  sl_gpio_set_pin_mode(&(sl_gpio_t){SL_ICM42688P_INT_PORT, SL_ICM42688P_INT_PIN},SL_GPIO_MODE_INPUT_PULL,true);  // true = pull-up, false = pull-down

#if defined(_SILICON_LABS_32B_SERIES_2)
  /* Use EMLIB to configure external interrupt on rising edge */
  GPIO_ExtIntConfig((GPIO_Port_TypeDef)SL_ICM42688P_INT_PORT,
                    SL_ICM42688P_INT_PIN,
                    SL_ICM42688P_INT_PIN,
                    true,  /* rising */
                    false, /* falling */
                    true); /* enable */
#else
  /* For older series, also use GPIO_ExtIntConfig if available */
  GPIO_ExtIntConfig((GPIO_Port_TypeDef)SL_ICM42688P_INT_PORT,
                    SL_ICM42688P_INT_PIN,
                    SL_ICM42688P_INT_PIN,
                    true,
                    false,
                    true);
#endif

  /* Configure INT pin active polarity open-drain / active low if desired by writing INT_CONFIG */
  /* Set INT_CONFIG: active high default; set to active low/open-drain if board expects it.
     Here we use default: no special polarity (driver user can call sl_icm42688p_enable_interrupt(...) to customize). */

  return SL_STATUS_OK;
}

sl_status_t sl_icm42688p_deinit(void)
{
  /* Put device to sleep  */
  uint8_t reg = ICM42688P_PWR_MGMT0_ACCEL_MODE_OFF | ICM42688P_PWR_MGMT0_GYRO_MODE_STANDBY | ICM42688P_PWR_MGMT0_TEMP_DIS;
  sl_icm42688p_write_register(ICM42688P_REG_PWR_MGMT0, reg);
  return SL_STATUS_OK;
}

sl_status_t sl_icm42688p_reset(void)
{
    uint8_t reset = 0x01; // Set SOFT_RESET_CONFIG bit
    sl_icm42688p_write_register(ICM42688P_REG_DEVICE_CONFIG, reset);
    sl_sleeptimer_delay_millisecond(1); // Wait at least 1 ms for reset to take effect
    return SL_STATUS_OK;
}

/* ----- Register read/write ----- */
sl_status_t sl_icm42688p_read_register(uint8_t reg, uint8_t *data, uint16_t len)
{
  sl_icm42688p_chip_select_set(true);
  sl_icm42688p_hw_delay_short();

#if defined(_SILICON_LABS_32B_SERIES_2)
  EUSART_Spi_TxRx(SL_ICM42688P_SPI_EUSART_PERIPHERAL, (uint8_t)(reg | 0x80U));
  for (uint16_t i = 0; i < len; ++i) {
    data[i] = EUSART_Spi_TxRx(SL_ICM42688P_SPI_EUSART_PERIPHERAL, 0x00U);
  }
#else
  sl_hal_eusart_spi_tx_rx(SL_ICM42688P_SPI_EUSART_PERIPHERAL, (uint8_t)(reg | 0x80U));
  for (uint16_t i = 0; i < len; ++i) {
    data[i] = sl_hal_eusart_spi_tx_rx(SL_ICM42688P_SPI_EUSART_PERIPHERAL, 0x00U);
  }
#endif

  sl_icm42688p_chip_select_set(false);
  sl_icm42688p_hw_delay_short();

  return SL_STATUS_OK;
}

sl_status_t sl_icm42688p_write_register(uint8_t reg, uint8_t data)
{
  sl_icm42688p_chip_select_set(true);
  sl_icm42688p_hw_delay_short();

#if defined(_SILICON_LABS_32B_SERIES_2)
  EUSART_Spi_TxRx(SL_ICM42688P_SPI_EUSART_PERIPHERAL, (uint8_t)(reg & 0x7FU));
  EUSART_Spi_TxRx(SL_ICM42688P_SPI_EUSART_PERIPHERAL, data);
#else
  sl_hal_eusart_spi_tx_rx(SL_ICM42688P_SPI_EUSART_PERIPHERAL, (uint8_t)(reg & 0x7FU));
  sl_hal_eusart_spi_tx_rx(SL_ICM42688P_SPI_EUSART_PERIPHERAL, data);
#endif

  sl_icm42688p_chip_select_set(false);
  sl_icm42688p_hw_delay_short();

  return SL_STATUS_OK;
}

sl_status_t sl_icm42688p_masked_write(uint8_t addr, uint8_t data, uint8_t mask)
{
    uint8_t reg;
    sl_status_t status;

    status = sl_icm42688p_read_register(addr, &reg, 1);
    if (status != SL_STATUS_OK) {
        return status;
    }

    reg &= ~mask;   // clear bits to be modified
    reg |= (data & mask);  // set new value

    status = sl_icm42688p_write_register(addr, reg);
    return status;
}

/* ----- Register bank selection -----*/
sl_status_t sl_icm42688p_set_bank(uint8_t bank)
{
    uint8_t val = bank & 0x07; // Only bits 2:0 are valid
    return sl_icm42688p_write_register(ICM42688P_REG_BANK_SEL, val);
}

/* ----- FS & ODR configuration ----- */
sl_status_t sl_icm42688p_set_full_scale_accel(uint8_t fs_code)
{
  sl_icm42688p_set_bank(ICM42688P_BANK_1);
  uint8_t reg = 0;
  /* FS code expected 0..7, we place into FS bits (shift) */
  reg = (uint8_t)((fs_code << ICM42688P_ACCEL_CONFIG0_SHIFT_FS_SEL) & ICM42688P_ACCEL_CONFIG0_MASK_FS_SEL);
  /* default to 1 kHz */
  reg |= (ICM42688P_ODR_CODE_1KHZ & 0x0FU);
  sl_icm42688p_write_register(ICM42688P_REG_ACCEL_CONFIG0, reg);
  sl_icm42688p_set_bank(ICM42688P_BANK_0);
  return SL_STATUS_OK;
}

sl_status_t sl_icm42688p_set_full_scale_gyro(uint8_t fs_code)
{
  sl_icm42688p_set_bank(ICM42688P_BANK_1);
  uint8_t reg = 0;
  reg = (uint8_t)((fs_code << ICM42688P_GYRO_CONFIG0_SHIFT_FS_SEL) & ICM42688P_GYRO_CONFIG0_MASK_FS_SEL);
  reg |= (ICM42688P_GYRO_ODR_1KHZ & 0x0FU);
  sl_icm42688p_write_register(ICM42688P_REG_GYRO_CONFIG0, reg);
  sl_icm42688p_set_bank(ICM42688P_BANK_0);
  return SL_STATUS_OK;
}

float sl_icm42688p_set_sample_rate(float sample_rate)
{
  uint8_t odr_code = (sample_rate >= 1000.0f) ? ICM42688P_ODR_CODE_1KHZ : ICM42688P_ODR_CODE_200HZ;

  sl_icm42688p_set_bank(ICM42688P_BANK_1);
  uint8_t reg = 0;

  /* Update gyroscope ODR */
  sl_icm42688p_read_register(ICM42688P_REG_GYRO_CONFIG0, &reg, 1);
  reg = (uint8_t)((reg & 0xF0U) | (odr_code & 0x0FU));
  sl_icm42688p_write_register(ICM42688P_REG_GYRO_CONFIG0, reg);

  /* Update accelerometer ODR */
  sl_icm42688p_read_register(ICM42688P_REG_ACCEL_CONFIG0, &reg, 1);
  reg = (uint8_t)((reg & 0xF0U) | (odr_code & 0x0FU));
  sl_icm42688p_write_register(ICM42688P_REG_ACCEL_CONFIG0, reg);

  sl_icm42688p_set_bank(ICM42688P_BANK_0);

  return (odr_code == ICM42688P_ODR_CODE_1KHZ) ? 1000.0f : 200.0f;
}

/* ----- Enable/disable sensors & interrupts ----- */
sl_status_t sl_icm42688p_enable_sensor(bool accel, bool gyro, bool temp)
{
  uint8_t reg = 0;

  reg |= accel ? ICM42688P_PWR_MGMT0_ACCEL_MODE_LOWNOISE : ICM42688P_PWR_MGMT0_ACCEL_MODE_OFF;
  reg |= gyro  ? ICM42688P_PWR_MGMT0_GYRO_MODE_LOWNOISE  : ICM42688P_PWR_MGMT0_GYRO_MODE_OFF;

  if (!temp) {
    reg |= ICM42688P_PWR_MGMT0_TEMP_DIS;
  }

  sl_icm42688p_write_register(ICM42688P_REG_PWR_MGMT0, reg);
  return SL_STATUS_OK;
}

sl_status_t sl_icm42688p_enable_interrupt(bool data_ready_enable)
{
    uint8_t int_cfg = 0x00U;   // Default: pulsed, open-drain, active-low for both INT1/INT2
    uint8_t int_enable = 0x00U; // All interrupts disabled by default

    /* Configure INT_CONFIG register for polarity and drive type */
    // Example: INT1 active-high, push-pull, pulsed
    int_cfg |= (1U << 0); // INT1_POLARITY = 1 → active high
    int_cfg |= (1U << 1); // INT1_DRIVE_CIRCUIT = 1 → push-pull
    // Leave INT1_MODE = 0 → pulsed mode
    // INT2 can be left as default (pulsed, open-drain, active-low)

    sl_icm42688p_write_register(ICM42688P_REG_INT_CONFIG, int_cfg);

    /* Enable Data Ready interrupt if requested */
    if (data_ready_enable) {
        int_enable |= ICM42688P_INT_STATUS0_DATA_RDY;  // Set mask for Data Ready interrupt
    }

    /* Masked write to INT_SOURCE0 register to enable DATA_RDY interrupt */
    sl_icm42688p_masked_write(ICM42688P_REG_INT_SOURCE0, int_enable, ICM42688P_INT_SOURCE0_UI_DRDY_INT1_EN);

    return SL_STATUS_OK;
}

sl_status_t sl_icm42688p_accel_read_data(float accel[3])
{
    uint8_t raw_data[6];
    float accel_res = ICM42688P_ACCEL_SCALE_16G; // Current resolution
    int16_t temp;

    /* Read the six raw data registers into the data array */
    sl_icm42688p_read_register(ICM42688P_REG_ACCEL_DATA_X1, raw_data, 6);

    /* Convert the MSB and LSB into a signed 16-bit value and multiply by the resolution to get the g value */
    temp = ((int16_t)raw_data[0] << 8) | raw_data[1];
    accel[0] = (float)temp * accel_res;

    temp = ((int16_t)raw_data[2] << 8) | raw_data[3];
    accel[1] = (float)temp * accel_res;

    temp = ((int16_t)raw_data[4] << 8) | raw_data[5];
    accel[2] = (float)temp * accel_res;

    return SL_STATUS_OK;
}

sl_status_t sl_icm42688p_gyro_read_data(float gyro[3])
{
    uint8_t raw_data[6];
    float gyro_res = ICM42688P_GYRO_SCALE_2000DPS; // Current resolution
    int16_t temp;

    /* Read the six raw data registers into the data array */
    sl_icm42688p_read_register(ICM42688P_REG_GYRO_DATA_X1, raw_data, 6);

    /* Convert the MSB and LSB into a signed 16-bit value and multiply by the resolution to get the dps value */
    temp = ((int16_t)raw_data[0] << 8) | raw_data[1];
    gyro[0] = (float)temp * gyro_res;

    temp = ((int16_t)raw_data[2] << 8) | raw_data[3];
    gyro[1] = (float)temp * gyro_res;

    temp = ((int16_t)raw_data[4] << 8) | raw_data[5];
    gyro[2] = (float)temp * gyro_res;

    return SL_STATUS_OK;
}

sl_status_t sl_icm42688p_read_temperature(float *temperature)
{
  uint8_t raw[2] = {0};
  int16_t tmp = 0;

  if (!temperature) {
      return SL_STATUS_INVALID_PARAMETER;
  }

  sl_icm42688p_read_register(ICM42688P_REG_TEMP_DATA1, raw, 2);
  tmp = (int16_t)((raw[0] << 8) | raw[1]);

  *temperature = ((float)tmp / ICM42688P_TEMP_SENSITIVITY) + ICM42688P_TEMP_OFFSET;
  return SL_STATUS_OK;
}

sl_status_t sl_icm42688p_get_device_id(uint8_t *dev_id)
{
  sl_icm42688p_read_register(ICM42688P_REG_WHO_AM_I, dev_id, 1);
  return SL_STATUS_OK;
}

bool sl_icm42688p_is_data_ready(void)
{
  uint8_t s = 0;
  sl_icm42688p_read_register(ICM42688P_REG_INT_STATUS0, &s, 1);
  return (s & ICM42688P_INT_STATUS0_DATA_RDY) != 0;
}

sl_status_t sl_icm42688p_accel_set_bandwidth(uint8_t odr_code)
{
    uint8_t reg;
    sl_icm42688p_read_register(ICM42688P_REG_ACCEL_CONFIG0, &reg, 1);
    reg = (reg & ~ICM42688P_ACCEL_ODR_MASK) | (odr_code & ICM42688P_ACCEL_ODR_MASK);
    sl_icm42688p_write_register(ICM42688P_REG_ACCEL_CONFIG0, reg);
    return SL_STATUS_OK;
}


sl_status_t sl_icm42688p_gyro_set_bandwidth(uint8_t odr_code)
{
    uint8_t reg;
    sl_icm42688p_read_register(ICM42688P_REG_GYRO_CONFIG0, &reg, 1);
    reg = (reg & ~ICM42688P_GYRO_ODR_MASK) | (odr_code & ICM42688P_GYRO_ODR_MASK);
    sl_icm42688p_write_register(ICM42688P_REG_GYRO_CONFIG0, reg);
    return SL_STATUS_OK;
}

sl_status_t sl_icm42688p_read_interrupt_status(uint32_t *status)
{
    uint8_t reg[2] = {0};

    if (!status) return SL_STATUS_INVALID_PARAMETER;

    sl_icm42688p_read_register(ICM42688P_REG_INT_STATUS0, reg, 2);

    *status = (uint32_t) reg[0] | ((uint32_t) reg[1] << 8);

    return SL_STATUS_OK;
}



sl_status_t sl_icm42688p_calibrate_gyro(float bias[3])
{
    if (!bias) return SL_STATUS_INVALID_PARAMETER;

    const int samples = 500;  // Number of samples for calibration
    float sum[3] = {0.0f, 0.0f, 0.0f};
    float gvec[3];

    for (int i = 0; i < samples; i++) {
        sl_icm42688p_gyro_read_data(gvec);  // Read raw gyro
        sum[0] += gvec[0];
        sum[1] += gvec[1];
        sum[2] += gvec[2];
        sl_sleeptimer_delay_millisecond(2);
    }

    bias[0] = sum[0] / samples;
    bias[1] = sum[1] / samples;
    bias[2] = sum[2] / samples;

    return SL_STATUS_OK;
}


sl_status_t sl_icm42688p_accel_get_resolution(float *accel_res)
{
    if (accel_res == NULL) {
        return SL_STATUS_INVALID_PARAMETER;
    }

    uint8_t fs_code = 0;
    sl_status_t status = sl_icm42688p_read_register(ICM42688P_REG_ACCEL_CONFIG0, &fs_code, 1);
    if (status != SL_STATUS_OK) {
        return status;
    }

    fs_code = (fs_code & ICM42688P_ACCEL_CONFIG0_MASK_FS_SEL) >> ICM42688P_ACCEL_CONFIG0_SHIFT_FS_SEL;

    switch (fs_code) {
        case 0:
            *accel_res = ICM42688P_ACCEL_SCALE_2G;
            break;
        case 1:
            *accel_res = ICM42688P_ACCEL_SCALE_4G;
            break;
        case 2:
            *accel_res = ICM42688P_ACCEL_SCALE_8G;
            break;
        case 3:
            *accel_res = ICM42688P_ACCEL_SCALE_16G;
            break;
        default:
            *accel_res = ICM42688P_ACCEL_SCALE_16G;
            break;
    }

    return SL_STATUS_OK;
}

sl_status_t sl_icm42688p_gyro_get_resolution(float *gyro_res)
{
    if (gyro_res == NULL) {
        return SL_STATUS_INVALID_PARAMETER;
    }

    uint8_t fs_code = 0;
    sl_status_t status = sl_icm42688p_read_register(ICM42688P_REG_GYRO_CONFIG0, &fs_code, 1);
    if (status != SL_STATUS_OK) {
        return status;
    }

    fs_code = (fs_code & ICM42688P_GYRO_CONFIG0_MASK_FS_SEL) >> ICM42688P_GYRO_CONFIG0_SHIFT_FS_SEL;

    switch (fs_code) {
        case 0:
          *gyro_res = ICM42688P_GYRO_SCALE_2000DPS;
          break;
        case 1:
          *gyro_res = ICM42688P_GYRO_SCALE_1000DPS;
          break;
        case 2:
          *gyro_res = ICM42688P_GYRO_SCALE_500DPS;
          break;
        case 3:
          *gyro_res = ICM42688P_GYRO_SCALE_250DPS;
          break;
        case 4:
          *gyro_res = ICM42688P_GYRO_SCALE_125DPS;
          break;
        case 5:
          *gyro_res = ICM42688P_GYRO_SCALE_62_5DPS;
          break;
        case 6:
          *gyro_res = ICM42688P_GYRO_SCALE_31_25DPS;
          break;
        case 7:
          *gyro_res = ICM42688P_GYRO_SCALE_15_625DPS;
          break;
        default:
          *gyro_res = ICM42688P_GYRO_SCALE_2000DPS;
          break;
    }

    return SL_STATUS_OK;
}


sl_status_t sl_icm42688p_calibrate_accel_and_gyro(float accel_bias[3], float gyro_bias[3])
{
    if (!accel_bias || !gyro_bias) {
        return SL_STATUS_INVALID_PARAMETER;
    }

    const uint16_t samples = 500;
    float accel_sum[3] = {0}, gyro_sum[3] = {0};
    float accel_data[3], gyro_data[3];

    // Ensure sensors are off before configuring
    sl_icm42688p_enable_sensor(false, false, false);

    // Set full-scale ranges
    sl_icm42688p_set_full_scale_accel(ICM42688P_ACCEL_CONFIG0_FS_2G >> ICM42688P_ACCEL_CONFIG0_SHIFT_FS_SEL);
    sl_icm42688p_set_full_scale_gyro(ICM42688P_GYRO_CONFIG0_FS_250DPS >> ICM42688P_GYRO_CONFIG0_SHIFT_FS_SEL);

    // Set sample rate to 1 kHz and bandwidth to 200 Hz
    sl_icm42688p_set_sample_rate(1000.0f);
    sl_icm42688p_accel_set_bandwidth(ICM42688P_ODR_CODE_200HZ);
    sl_icm42688p_gyro_set_bandwidth(ICM42688P_ODR_CODE_200HZ);

    // Enable accelerometer and gyroscope (temperature optional)
    sl_icm42688p_enable_sensor(true, true, true);

    // Small delay for sensors to stabilize
    sl_sleeptimer_delay_millisecond(50);

    // Collect samples
    for (uint16_t i = 0; i < samples; i++) {
        sl_icm42688p_accel_read_data(accel_data);
        sl_icm42688p_gyro_read_data(gyro_data);

        accel_sum[0] += accel_data[0];
        accel_sum[1] += accel_data[1];
        accel_sum[2] += accel_data[2];

        gyro_sum[0] += gyro_data[0];
        gyro_sum[1] += gyro_data[1];
        gyro_sum[2] += gyro_data[2];

        sl_sleeptimer_delay_millisecond(2);
    }

    // Compute average
    accel_bias[0] = accel_sum[0] / samples;
    accel_bias[1] = accel_sum[1] / samples;
    accel_bias[2] = accel_sum[2] / samples;

    gyro_bias[0] = gyro_sum[0] / samples;
    gyro_bias[1] = gyro_sum[1] / samples;
    gyro_bias[2] = gyro_sum[2] / samples;

    // Correct Z-axis for 1g offset
    if (accel_bias[2] > 0.0f) {
        accel_bias[2] -= ICM42688P_ACCEL_SCALE_2G;
    } else {
        accel_bias[2] += ICM42688P_ACCEL_SCALE_2G;
    }

    // Disable sensors after calibration
    sl_icm42688p_enable_sensor(false, false, false);

    return SL_STATUS_OK;
}


/* ----- Helpers ----- */
static void sl_icm42688p_chip_select_set(bool select)
{
  sl_gpio_t cs_pin = {
    .port = SL_ICM42688P_SPI_EUSART_CS_PORT,
    .pin  = SL_ICM42688P_SPI_EUSART_CS_PIN
  };

  if (select) {
    sl_gpio_clear_pin(&cs_pin);
  } else {
    sl_gpio_set_pin(&cs_pin);
  }
}


/* small delay for t_SCS/t_SCCS timing */
static void sl_icm42688p_hw_delay_short(void)
{
  for (volatile int i = 0; i < 8; ++i) {
    __NOP();
  }
}
