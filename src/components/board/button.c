#include "button.h"
#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_timer.h"
#include "hal/gpio_types.h"
#include "freertos/FreeRTOS.h" // IWYU pragma: keep
#include "esp_log.h" // IWYU pragma: keep
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "utils_module.h"

static const char* TAG = "Button";

static uint32_t last_isr_time = 0;
static uint32_t last_hold_time = 0;

bool button_is_cliked(){
    return gpio_get_level(BTN_GPIO);
}


static void btn_task(void* arg){
    bool hold = false;
    button_callback_t cb = (button_callback_t)arg;
    while (button_is_cliked()) {
        uint32_t current_time = US_TO_MS(esp_timer_get_time()) ;
        if (current_time - last_isr_time > BUTTON_HOLD_MS) {
            last_hold_time = current_time;
            if (cb) {
                cb(BUTTON_STATE_HOLD);
            }
            hold = true;
            break;
        }
    }
    if (!hold){
        if (cb) {
            cb(BUTTON_STATE_CLICKED);
        }
    }
    vTaskDelete(NULL);
}

static void IRAM_ATTR btn_isr_handler(void* arg){
    uint32_t current_time = US_TO_MS(esp_timer_get_time());
    if (current_time - last_isr_time > BUTTON_COOLDOWN_MS) {
        last_isr_time = current_time;
        xTaskCreate(btn_task, "btn_task", BUTTON_TASK_STACK_SIZE, arg, 10, NULL);
    }

}

esp_err_t button_init(button_callback_t callback){
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_POSEDGE,
        .pin_bit_mask = 1ULL<<BTN_GPIO,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = true
    };
    CHECK_ERROR(gpio_config(&io_conf), "Failed to configure button gpio");
    CHECK_ERROR(gpio_isr_handler_add(BTN_GPIO, btn_isr_handler, callback), "Failed to add button isr handler");
    return ESP_OK;
}
