#include <soc/gpio_num.h>
#include <time.h>
#include "board.h"
#include "driver/i2c_types.h"
#include "driver/spi_common.h"
#include "driver/i2c_master.h"
#include "ioe_tca6408a.h"
#include "touch_chsc6x.h"

void board_init_spi(void) {
    spi_bus_config_t spi_bus_conf = {
        .sclk_io_num = DISPLAY_SPI_CLK,
        .data0_io_num = DISPLAY_SPI_DATA0,
        .data1_io_num = DISPLAY_SPI_DATA1,
        .data2_io_num = DISPLAY_SPI_DATA2,
        .data3_io_num = DISPLAY_SPI_DATA3,
        .max_transfer_sz = LCD_H_RES * LCD_V_RES * LCD_BIT_PER_PIXEL / 8,
    };
    ESP_ERROR_CHECK(spi_bus_initialize(DISPLAY_SPI_HOST, &spi_bus_conf, SPI_DMA_CH_AUTO));
}

i2c_master_bus_handle_t board_init_i2c(void) {
    i2c_master_bus_config_t i2c_bus_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = 0,
        .scl_io_num = SCL_GPIO,
        .sda_io_num = SDA_GPIO,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };
    i2c_master_bus_handle_t i2c_bus_handle;
    ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_bus_config, &i2c_bus_handle));
    return i2c_bus_handle;
}

void init_ioe_pins_direton(){
    ioe_set_reg_pin(ioe_reg_direction, 1, ioe_dir_output); //Set IOE pin 1 as output
}

void board_init(void) {
    board_init_spi();
    i2c_master_bus_handle_t i2c_bus_handle = board_init_i2c();
    ioe_init(i2c_bus_handle);
    touch_init(i2c_bus_handle);
}
