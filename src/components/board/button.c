#include "button.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "hal/gpio_types.h"
#include "freertos/FreeRTOS.h" // IWYU pragma: keep
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "esp_sleep.h"

static uint32_t last_isr_time = 0;
static uint32_t last_hold_time = 0;


bool button_is_cliked(){
    return gpio_get_level(BTN_GPIO);
}


static void btn_task(void* arg){
    bool hold = false;
    button_callback_t cb = (button_callback_t)arg;
    while (button_is_cliked()) {
        uint32_t current_time = esp_log_timestamp();
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
    uint32_t current_time = esp_log_timestamp();
    if (current_time - last_isr_time > BUTTON_COOLDOWN_MS) {
        last_isr_time = current_time;
        xTaskCreate(btn_task, "btn_task", BUTTON_TASK_STACK_SIZE, arg, 10, NULL);
    }

}

void button_init(button_callback_t callback){
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_POSEDGE,
        .pin_bit_mask = 1ULL<<BTN_GPIO,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = true
    };
    gpio_config(&io_conf);
    gpio_isr_handler_add(BTN_GPIO, btn_isr_handler, callback);
}
