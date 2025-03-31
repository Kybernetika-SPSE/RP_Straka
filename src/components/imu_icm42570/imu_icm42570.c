/*
This code serves as an extension of Espressif's ICM42670 driver.
It provides additional functionality for our specific use case, such as APEX.
*/

#include "imu_icm42570.h"
#include "esp_err.h"
#include "esp_rom_sys.h"
#include "freertos/idf_additions.h"
#include "icm42670.h"
#include <string.h>
#include "esp_timer.h"
#include "esp_log.h"
#include "utils_module.h"

/*
This part of the code is taken from Espressif's ICM42670 driver.
Since this code serves as a purpose specific extension of their driver
it wouldn't be fit for merging into upstream

Copyright 2023-2025 Espressif Systems (Shanghai) CO LTD

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

typedef struct {
    i2c_master_dev_handle_t i2c_handle;
    uint32_t counter;
    float dt;  /*!< delay time between two measurements, dt should be small (ms level) */
    struct timeval *timer;
} icm42670_dev_t;

// These functions have been modified to use `icm42670_dev_t` instead of `icm42670_handle_t`

static esp_err_t icm42670_write(icm42670_dev_t *sens, const uint8_t reg_start_addr, const uint8_t *data_buf, const uint8_t data_len)
{
    assert(data_len < 5);
    uint8_t write_buff[5] = {reg_start_addr};
    memcpy(&write_buff[1], data_buf, data_len);
    return i2c_master_transmit(sens->i2c_handle, write_buff, data_len + 1, -1);
}

static esp_err_t icm42670_read(icm42670_dev_t *sens, const uint8_t reg_start_addr, uint8_t *data_buf, const uint8_t data_len)
{
    uint8_t reg_buff[] = {reg_start_addr};

    /* Write register number and read data */
    return i2c_master_transmit_receive(sens->i2c_handle, reg_buff, sizeof(reg_buff), data_buf, data_len, -1);
}
/*End of Espressif's code*/

// Used to write to bank 1 registers
static esp_err_t icm42670_write_bank1(icm42670_dev_t *sens, uint8_t reg_addr, uint8_t data_buf)
{
    return icm42670_write(icm42670, ICM42670_REG_BLK_SEL_W, (uint8_t[]){ICM42670_SELECT_BANK_MREG1, reg_addr, data_buf}, 3);
}

// Used to write to bank 1 registers
static esp_err_t icm42670_read_bank1(icm42670_dev_t *sens, uint8_t reg_addr, uint8_t *data_buf)
{
    icm42670_write(icm42670, ICM42670_REG_BLK_SEL_R, (uint8_t[]){ICM42670_SELECT_BANK_MREG1, reg_addr}, 2);
    esp_rom_delay_us(10);
    return icm42670_read(icm42670, ICM42670_REG_M_R, data_buf, 1);
}

// Testing code for DPM bringup
/*
void imu_task_wait_for_dmp_init(void *pvParameters){
    uint8_t apex_config0;
    uint8_t mclk_rdy;

    while (true){
        icm42670_read(icm42670, ICM42670_REG_APEX_CONFIG0, &apex_config0, 1);
        icm42670_read(icm42670, ICM42670_REG_MCLK_RDY, &mclk_rdy, 1);
        if (!(apex_config0 & ICM42670_DMP_INIT_EN) && (mclk_rdy == ICM42670_MCLK_RDY)){
            icm42670_write(icm42670, ICM42670_REG_APEX_CONFIG1, (uint8_t[]){ICM42670_SMD_ENABLE | ICM42670_TILT_ENABLE | IMU_DMP_ODR_50HZ}, 1); // Enable SMD
            icm42670_write(icm42670, ICM42670_REG_INT_SOURCE1, (uint8_t[]){ICM42670_INT_ENABLE_SMD}, 1); // enable SMD interrupt
            icm42670_write_bank1(icm42670, ICM42670_REG_INT_SOURCE6, ICM42670_TILT_DET_EN); // enable TILT interrupt
            icm42670_write_bank1(icm42670, ICM42670_REG_APEX_CONFIG5, IMU_TILT_WAIT_TIME_0); // make TILT time smaller
            icm42670_write_bank1(icm42670, ICM42670_REG_APEX_CONFIG9, IMU_SPD_SENSITIVITY_0); // highest sensitivity
            vTaskDelete(NULL);
        }
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

void imu_setup_apex(){
    icm42670_write(icm42670, ICM42670_REG_APEX_CONFIG0, (uint8_t[]){ICM42670_DMP_INIT_EN}, 1);
    xTaskCreate(imu_task_wait_for_dmp_init, "IMU DSP init task", 4096, NULL, 5, NULL);
}
*/

void imu_init(i2c_master_bus_handle_t bus_handle){
    icm42670_create(bus_handle, ICM42670_I2C_ADDRESS_1, &icm42670);
    // icm42670_write(icm42670, ICM42670_REG_SIGNAL_PATH_RESET, (uint8_t[]){ICM42670_SOFT_RESET_DEVICE_CONFIG}, 1);
    icm42670_config(icm42670, &imu_cfg);
    icm42670_acce_set_pwr(icm42670, ACCE_PWR_LOWNOISE);
    icm42670_gyro_set_pwr(icm42670, GYRO_PWR_LOWNOISE);
}

void imu_setup_apex(){
    icm42670_write(icm42670, ICM42670_REG_INT_SOURCE1, (uint8_t[]){ICM42670_INT_ENABLE_WOM_Y}, 1); // enable WOM interrupt on INT1
    icm42670_write(icm42670, ICM42670_REG_INT_CONFIG, (uint8_t[]){0b111}, 1); // set interrupt polarity to active high, pushpull and latched
    icm42670_write_bank1(icm42670, ICM42670_REG_ACCEL_WOM_Y_THR, 200); // 0g - 0, 0.5g - 128, 1g - 255
    icm42670_write(icm42670, ICM42670_REG_WOM_CONFIG, (uint8_t[]){ICM42670_WOM_EN}, 1); // enable WOM
}

void imu_read_acc(icm42670_value_t *acc){
    icm42670_get_acce_value(icm42670, acc);
}

void imu_read_gyro(icm42670_value_t *gyro){
    icm42670_get_gyro_value(icm42670, gyro);
}

void imu_read_temp(float *temperature){
    icm42670_get_temp_value(icm42670, temperature);
}

// Debugging functions
void imu_poll(){
    icm42670_value_t acc_data;
    icm42670_value_t gyro_data;

    imu_read_acc(&acc_data);
    imu_read_gyro(&gyro_data);

    uint8_t int_status[3];
    icm42670_read(icm42670, ICM42670_REG_INT_STATUS1, int_status, sizeof(int_status));

    ESP_LOGI("IMU", "Accelerometer: X %f, Y %f, Z %f", acc_data.x, acc_data.y, acc_data.z);
    ESP_LOGI("IMU", "Gyroscope: X %f, Y %f, Z %f", gyro_data.x, gyro_data.y, gyro_data.z);
    ESP_LOGI("IMU", "INT_STATUS1: %d, INT_STATUS2: %d, INT_STATUS3: %d", int_status[0], int_status[1], int_status[2]);
}

void imu_setup_debug_polling(){
    const esp_timer_create_args_t imu_poll_timer_args = {
        .callback = imu_poll,
        .name = "imu_poll"
    };
    esp_timer_handle_t imu_poll_timer = NULL;
    ESP_ERROR_CHECK(esp_timer_create(&imu_poll_timer_args, &imu_poll_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(imu_poll_timer, MS_TO_US(1000)));
}
