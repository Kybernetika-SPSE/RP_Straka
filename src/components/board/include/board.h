#ifndef BOARD_H
#define BOARD_H

#define DISPLAY_SPI_HOST    SPI2_HOST

#define SCL_GPIO 21
#define SDA_GPIO 8
#define DISPLAY_SPI_CLK   6
#define DISPLAY_SPI_CS   10
#define DISPLAY_SPI_DATA0   2
#define DISPLAY_SPI_DATA1   7
#define DISPLAY_SPI_DATA2   5
#define DISPLAY_SPI_DATA3   4

#define LVGL_BUFFER 466*22*3

#define LCD_H_RES 466
#define LCD_V_RES 466

#define LCD_BIT_PER_PIXEL 24

void board_init(void);

#endif
