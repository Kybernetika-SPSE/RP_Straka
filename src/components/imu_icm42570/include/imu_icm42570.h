#ifndef IMU_ICM42570_H
#define IMU_ICM42570_H

#include "icm42670.h"

#define ICM42670_REG_MCLK_RDY 0
#define ICM42670_REG_SIGNAL_PATH_RESET 0x02

#define ICM42670_REG_INT_CONFIG 0x06

#define ICM42670_REG_APEX_CONFIG0 0x25
#define ICM42670_REG_APEX_CONFIG1 0x26

#define ICM42670_REG_WOM_CONFIG 0x27

#define ICM42670_REG_INT_SOURCE1 0x2C

#define ICM42670_REG_INT_SOURCE6 0x2F // MREG1

#define ICM42670_REG_INT_STATUS1 0x3A
#define ICM42670_REG_INT_STATUS2 0x3B
#define ICM42670_REG_INT_STATUS3 0x3C

#define ICM42670_REG_APEX_CONFIG5 0x47
#define ICM42670_REG_APEX_CONFIG9 0x48

#define ICM42670_REG_ACCEL_WOM_Y_THR 0x4C

#define ICM42670_REG_BLK_SEL_W 0x79
#define ICM42670_REG_BLK_SEL_R 0x7C
#define ICM42670_REG_M_R ICM42670_REG_BLK_SEL_R+2

#define ICM42670_SELECT_BANK_MREG1 0

#define ICM42670_SOFT_RESET_DEVICE_CONFIG 1 << 4 // SIGNAL_PATH_RESET - Reset IMU

#define ICM42670_MCLK_RDY (1 << 3)+1 // MCLK_RDY - MCLK online - +1 because of OTP load

#define ICM42670_DMP_MEM_RESET_EN 1  // APEX_CONFIG0
#define ICM42670_DMP_INIT_EN 1 << 2 // APEX_CONFIG0

#define ICM42670_TILT_ENABLE 1 << 4 // APEX_CONFIG1
#define ICM42670_SMD_ENABLE 1 << 6 // APEX_CONFIG1

#define ICM42670_INT_ENABLE_WOM_Y 1 << 1 // INT_SOURCE1
#define ICM42670_INT_ENABLE_SMD 1 << 3 // INT_SOURCE1

#define ICM42670_TILT_DET_EN 1 << 3 // INT_SOURCE6

#define ICM42670_WOM_EN 1 // WOM_CONFIG

enum IMU_SPD_SENSITIVITY_LEVELS {
    IMU_SPD_SENSITIVITY_0, // Most sensitive
    IMU_SPD_SENSITIVITY_1 = 0b1 << 1,
    IMU_SPD_SENSITIVITY_2 = 0b10 << 1,
    IMU_SPD_SENSITIVITY_3 = 0b11 << 1,
    IMU_SPD_SENSITIVITY_4 = 0b100 << 1 // Least sensitive
};

enum IMU_DMP_ODR {
    IMU_DMP_ODR_25HZ,
    IMU_DMP_ODR_400HZ,
    IMU_DMP_ODR_50HZ,
    IMU_DMP_ODR_100HZ
};

enum IMU_TILT_WAIT_TIME {
    IMU_TILT_WAIT_TIME_0,
    IMU_TILT_WAIT_TIME_2 = 0b01<<6,
    IMU_TILT_WAIT_TIME_4 = 0b10<<6,
    IMU_TILT_WAIT_TIME_8 = 0b11<<6
};

static icm42670_handle_t icm42670 = NULL;

const static icm42670_cfg_t imu_cfg = {
    .acce_fs = ACCE_FS_2G,
    .acce_odr = ACCE_ODR_400HZ,
    .gyro_fs = GYRO_FS_2000DPS,
    .gyro_odr = GYRO_ODR_400HZ,
}; // Initial values from Espressif's example, expected to change

void imu_init(i2c_master_bus_handle_t bus_handle);
void imu_setup_apex();
void imu_read_acc(icm42670_value_t *acc);
void imu_read_gyro(icm42670_value_t *gyro);
void imu_read_temp(float *temperature);
void imu_setup_debug_polling();
#endif
