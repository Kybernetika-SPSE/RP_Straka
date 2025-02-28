#include "amoled_co5300.h"
#include "co5300_registers.h"
#include "display/lv_display.h"
#include "driver/spi_master.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <time.h>
#include "amoled_co5300.h"
#include "ioe_tca6408a.h"
#include "utils_module.h"

static const char *TAG = "CO5300";

spi_device_handle_t spi_lcd_handle;


esp_err_t add_spi_CO5300_dev(spi_device_handle_t *spi_lcd_handle) {
    spi_device_interface_config_t lcd_interface_conf = {
        .spics_io_num = AMOLED_SPI_CS,
        .clock_speed_hz = SPI_MASTER_FREQ_80M,
        .mode = 0,
        .queue_size = 1, // Transaction queue size
        .command_bits = 8,
        .dummy_bits = 8, // zero bits between address and data
        .address_bits = 16,
        .flags = SPI_DEVICE_HALFDUPLEX
    };
    return spi_bus_add_device(DISPLAY_SPI_HOST, &lcd_interface_conf, spi_lcd_handle);
}

void display_hw_reset(){
    ioe_set_reg_pin(ioe_reg_output, 1, 0);
    vTaskDelay(pdMS_TO_TICKS(200));
    ioe_set_reg_pin(ioe_reg_output, 1, 1);
    vTaskDelay(pdMS_TO_TICKS(100));
    ioe_set_reg_pin(ioe_reg_output, 1, 0);
    vTaskDelay(pdMS_TO_TICKS(200));
    ioe_set_reg_pin(ioe_reg_output, 1, 1);
    vTaskDelay(pdMS_TO_TICKS(50));
}

esp_err_t display_send_spi(uint8_t cmd, uint8_t addr, uint8_t* data, size_t data_len) {
    spi_transaction_t t = {
        .cmd = cmd,
        .addr = addr,
        .tx_buffer = data,
        .length = (data != NULL ? data_len * 8 : 0),
    };
    return spi_device_polling_transmit(spi_lcd_handle, &t);;
}

esp_err_t display_send_pixels(uint8_t* data, size_t data_len) {
    spi_transaction_t t = {
        .cmd = CMD_WRITE_QUAD_SPI_24CLK,
        .addr = ADR_START_PIXEL_WRITE,
        .tx_buffer = data,
        .length = (data != NULL ? data_len * 8 : 0),
        .flags = SPI_TRANS_MODE_QIO,
    };
    CHECK_ERROR(spi_device_queue_trans(spi_lcd_handle, &t, portMAX_DELAY), "Failed to send pixels");
    return ESP_OK;
}

esp_err_t display_minimal_init(void) {
    return add_spi_CO5300_dev(&spi_lcd_handle);
}

esp_err_t display_full_init(void) {
    CHECK_ERROR(display_minimal_init(), "Failed to minimal init display");
    display_hw_reset();
    INIT_CMDS(display_send_spi);
    ESP_LOGI(TAG, "Init done!");
    return ESP_OK;
}


esp_err_t display_set_draw_zone(uint16_t Xstart, uint16_t Xend, uint16_t Ystart, uint16_t Yend) {
	Xstart=Xstart+0x06;
	Xend=Xend+0x06;
	uint8_t column_data[] = {Xstart>>8, Xstart&0xff, Xend>>8, Xend&0xff};
	uint8_t row_data[] = {Ystart>>8, Ystart&0xff, Yend>>8, Yend&0xff};
	CHECK_ERROR(display_send_spi(CMD_WRITE, ADR_SET_COLUMN_START, column_data, 4), "Failed to set column draw zone");
	CHECK_ERROR(display_send_spi(CMD_WRITE, ADR_SET_ROW_START, row_data, 4), "Failed to set row draw zone");
	return ESP_OK;
}

void display_flush_cb(lv_display_t * display, const lv_area_t * area, uint8_t * px_map){
    size_t writesize = (area->x2-area->x1)*(area->y2-area->y1+2)*3;
    display_set_draw_zone(area->x1, area->x2, area->y1, area->y2);
    display_send_pixels(px_map, writesize);
    lv_display_flush_ready(display);
}

esp_err_t display_sleep(void){
    CHECK_ERROR(display_send_spi(CMD_WRITE, ADR_SLEEP_IN, NULL, 0), "Failed to sleep display");
    return ESP_OK;
}

esp_err_t display_wakeup(void){
    CHECK_ERROR(display_send_spi(CMD_WRITE, ADR_SLEEP_OUT, NULL, 0), "Failed to sleep display");
    return ESP_OK;
}
