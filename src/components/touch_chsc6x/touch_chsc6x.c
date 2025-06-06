#include "touch_chsc6x.h"
#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include "esp_log.h" // IWYU pragma: keep
#include "hal/gpio_types.h"
#include "freertos/FreeRTOS.h" // IWYU pragma: keep
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "utils_module.h"

static const char* TAG = "Touch";

static void (*touch_callback)(void *) = NULL;
static void* touch_callback_args = NULL;

static i2c_master_dev_handle_t touch_dev_handle;
static touch_data_t touch_buf; // touch can be as array of touches
static SemaphoreHandle_t touch_mutex;
static touch_data_t last_touch = {0,0,0};

touch_data_t touch_calculate_cords(uint8_t* buff){
    touch_data_t touch_data = {0,0,false};
    uint8_t sector_byte = buff[0];
    uint8_t x = buff[1];
    uint8_t y = buff[2];
    touch_data.pressed = sector_byte & TOUCH_PRESSED_MASK;
    uint8_t sector_base = sector_byte & SECTOR_BASE_MASK;
    int sector_index = (sector_base - SECTOR_OFFSET) / SECTOR_STEP;
    bool is_high_x = (sector_index & HIGH_X_INDEX_MASK);
    bool is_high_y = (sector_index & HIGH_Y_INDEX_MASK);
    touch_data.x = (is_high_x * 0xFF) + x;
    touch_data.y = (is_high_y * 0xFF) + y;
    return touch_data;
}

touch_data_t touch_get_data(void){
    uint8_t rx_buf[TOUCH_READ_POINT_LEN];
    i2c_master_receive(touch_dev_handle, rx_buf, TOUCH_READ_POINT_LEN, -1);
    return touch_calculate_cords(rx_buf);
}

bool are_touches_same(touch_data_t touchA, touch_data_t touchB){
    return (touchA.x == touchB.x && touchA.y == touchB.y && touchA.pressed == touchB.pressed);
}

bool touch_int_is_up(){
    return !gpio_get_level(TOUCH_INT_GPIO);
}

static void touch_task(void* arg){
    if(touch_int_is_up()){
        touch_data_t touch_now = touch_get_data();
        if (!are_touches_same(last_touch, touch_now)){
            xSemaphoreTake(touch_mutex, portMAX_DELAY);
            touch_buf = touch_now;
            if (touch_callback != NULL){
                touch_callback(touch_callback_args);
            }
            xSemaphoreGive(touch_mutex);
        }
    }
    vTaskDelete(NULL);
}

static void IRAM_ATTR touch_isr_handler(void* arg){
    xTaskCreate(touch_task, "touch_task", TOUCH_TASK_STACK_SIZE, NULL, 10, NULL);
}

void touch_lvgl_cb (lv_indev_t * indev, lv_indev_data_t * data){
    xSemaphoreTake(touch_mutex, portMAX_DELAY);
    touch_data_t touch_data = touch_buf;
    data->point.x = touch_data.x;
    data->point.y = touch_data.y;
    data->state = touch_data.pressed;
    xSemaphoreGive(touch_mutex);
}

esp_err_t init_i2c_dev(i2c_master_bus_handle_t bus_handle){
    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = TOUCH_I2C_ADDRESS,
        .scl_speed_hz = TOUCH_I2C_CLOCK,
    };
    return i2c_master_bus_add_device(bus_handle, &dev_cfg, &touch_dev_handle);
}

esp_err_t create_touch_mutex(){
    touch_mutex = xSemaphoreCreateMutex();
    if (touch_mutex == NULL) {
        return ESP_FAIL;
    }
    return ESP_OK;
}

esp_err_t touch_conf_gpio(){
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_POSEDGE,
        .pin_bit_mask = 1ULL<<TOUCH_INT_GPIO,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = true
    };
    return gpio_config(&io_conf);
}

esp_err_t touch_init_gpio_int(){
    CHECK_ERROR(touch_conf_gpio(), "Failed to configure touch gpio");
    CHECK_ERROR(gpio_set_intr_type(TOUCH_INT_GPIO, GPIO_INTR_ANYEDGE), "Failed to set touch gpio intr type");
    CHECK_ERROR(gpio_isr_handler_add(TOUCH_INT_GPIO, touch_isr_handler, 0), "Failed to add touch isr handler");
    return ESP_OK;
}

esp_err_t touch_init(i2c_master_bus_handle_t bus_handle) {
    CHECK_ERROR(init_i2c_dev(bus_handle), "Failed to init touch i2c device");
    CHECK_ERROR(create_touch_mutex(), "Failed to create touch mutex");
    CHECK_ERROR(touch_init_gpio_int(), "Failed to init touch gpio int");
    return ESP_OK;
}


void touch_set_callback(void (*cb)(), void* args){
    touch_callback = cb;
    touch_callback_args = args;
}
