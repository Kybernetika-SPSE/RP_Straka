#ifndef Peripherals_h
#define Peripherals_h

#include "driver/i2c_types.h"
#include "esp_err.h"


#define SCL_GPIO 21
#define SDA_GPIO 8

#define BOARD_SPI_HOST SPI2_HOST

#define BOARD_SPI_CLK 6
#define BOARD_SPI_DATA0 2
#define BOARD_SPI_DATA1 7
#define BOARD_SPI_DATA2 5
#define BOARD_SPI_DATA3 4

#define BOARD_INT2_IOE 1

#define LCD_H_RES 466
#define LCD_V_RES 466

#define LCD_BIT_PER_PIXEL 24

esp_err_t board_init_spi(void);
esp_err_t board_init_i2c(i2c_master_bus_handle_t* i2c_bus_handle);
esp_err_t init_nvs();

#endif
