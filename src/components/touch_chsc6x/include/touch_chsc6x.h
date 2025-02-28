#ifndef TOUCH_CHSC6X_H
#define TOUCH_CHSC6X_H
#include "driver/i2c_types.h"
#include "esp_err.h"
#include "indev/lv_indev.h"

struct TouchHardwareSettings{
    #define TOUCH_INT_GPIO GPIO_NUM_0
    #define TOUCH_I2C_ADDRESS 0x2E
    #define TOUCH_I2C_CLOCK 100000
};
struct TouchCommunication{
    #define TOUCH_READ_LEN 5
    #define ADR_READ_CORDS 0x5D
    #define TOUCH_READ_POINT_LEN 3
    #define TOUCH_BUF_COUNT 64
};
struct TouchCanculation{
    #define TOUCH_PRESSED_MASK 0x01
    #define SECTOR_BASE_MASK 0xF0
    #define SECTOR_OFFSET 0x10
    #define SECTOR_STEP 0x40
    #define HIGH_X_INDEX_MASK 0b01
    #define HIGH_Y_INDEX_MASK 0b10
    #define LOW_CORD_MAX_VAL 0b10
    #define COORD_HIGH_BYTE_VALUE 0xFF
};

#define TOUCH_TASK_STACK_SIZE 4096

struct{
    int x;
    int y;
    bool pressed;
} typedef touch_data_t;

esp_err_t touch_init(i2c_master_bus_handle_t bus_handle);
void touch_set_callback(void (*cb)(), void* args);
touch_data_t touch_get_data(void);
void touch_lvgl_cb (lv_indev_t * indev, lv_indev_data_t * data);
#endif
