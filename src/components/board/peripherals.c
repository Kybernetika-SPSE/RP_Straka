#include "driver/spi_common.h"
#include "driver/i2c_master.h"
#include "esp_err.h"
#include "esp_log.h" // IWYU pragma: keep
#include "nvs_flash.h"
#include "utils_module.h"
#include "peripherals.h"

static const char* TAG = "Peripherals";

esp_err_t board_init_spi(void) {
    spi_bus_config_t spi_bus_conf = {
        .sclk_io_num = BOARD_SPI_CLK,
        .data0_io_num = BOARD_SPI_DATA0,
        .data1_io_num = BOARD_SPI_DATA1,
        .data2_io_num = BOARD_SPI_DATA2,
        .data3_io_num = BOARD_SPI_DATA3,
        .max_transfer_sz = LCD_H_RES * LCD_V_RES * LCD_BIT_PER_PIXEL / 8,
    };
    return spi_bus_initialize(BOARD_SPI_HOST, &spi_bus_conf, SPI_DMA_CH_AUTO);
}

esp_err_t board_init_i2c(i2c_master_bus_handle_t* i2c_bus_handle) {
    i2c_master_bus_config_t i2c_bus_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = 0,
        .scl_io_num = SCL_GPIO,
        .sda_io_num = SDA_GPIO,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };
    return i2c_new_master_bus(&i2c_bus_config, i2c_bus_handle);
}

esp_err_t init_nvs(){
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        CHECK_ERROR(nvs_flash_erase(), "Failed to erase nvs");
        ret = nvs_flash_init();
    }
    return ret;
}
