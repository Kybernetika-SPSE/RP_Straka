#ifndef AMOLED_CO5300_H
#define AMOLED_CO5300_H

#include "co5300_registers.h"
#include "driver/spi_master.h"
#include "misc/lv_area.h"
#include "misc/lv_types.h"

extern spi_device_handle_t spi_lcd_handle;

#define DELAY_MS(ms) vTaskDelay(pdMS_TO_TICKS(ms))

#define INIT_CMDS(send) \
    do { \
        send(CMD_WRITE, ADR_SELECT_PAGE, (uint8_t[]){0x00}, 1); \
        send(CMD_WRITE, ADR_CONTROL_SRAM, (uint8_t[]){0x80}, 1); \
        send(CMD_WRITE, ADR_SET_PIXEL_FORMAT, (uint8_t[]){0x77}, 1); \
        send(CMD_WRITE, ADR_CONTROL_TERING_PIN, (uint8_t[]){0x00}, 1); \
        send(CMD_WRITE, ADR_WRITE_CTRL, (uint8_t[]){0x20}, 1); \
        send(CMD_WRITE, ADR_SET_BRIGHTNESS, (uint8_t[]){0x00}, 1); \
        send(CMD_WRITE, ADR_WRITE_HBM, (uint8_t[]){0x00}, 1); \
        send(CMD_WRITE, ADR_SET_COLUMN_START, (uint8_t[]){0x00, 0x08, 0x01, 0xD9}, 4); \
        send(CMD_WRITE, ADR_SET_ROW_START, (uint8_t[]){0x00, 0x00, 0x01, 0xD1}, 4); \
        send(CMD_WRITE, ADR_SET_SCAN_DIRECTION, (uint8_t[]){0b00001000}, 1); \
        send(CMD_WRITE, ADR_SLEEP_OUT, NULL, 0); \
        DELAY_MS(120); \
        send(CMD_WRITE, ADR_DISPLAY_ON, NULL, 0); \
        send(CMD_WRITE, ADR_SET_BRIGHTNESS, (uint8_t[]){0x66}, 1); \
    } while (0)
// TODO: rewrite ADR_SET_COLUMN_START setting of display size and offset

void init_CO5300(void);
void display_flush_cb(lv_display_t * display, const lv_area_t * area, uint8_t * px_map);
void display_set_draw_zone(uint16_t Xstart, uint16_t Xend, uint16_t Ystart, uint16_t Yend);
void display_send_pixels(uint8_t* data, size_t data_len);
#endif
