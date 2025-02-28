#include "sleep.h"
#include "amoled_co5300.h"
#include "ble_module.h"
#include "esp_err.h"
#include "esp_timer.h"
#include "utils_module.h"
#include "button.h"
#include "esp_sleep.h"
#include "esp_log.h" // IWYU pragma: keep
#include "touch_chsc6x.h"

static const char* TAG = "Sleep";

static int sleep_time_ms = DEFAULT_SLEEP_TIMER_PERIOD_SECONDS;
static esp_timer_handle_t sleep_enter_timer = NULL;

void reset_sleep_enter_timer(){
    esp_timer_restart(sleep_enter_timer, SECONDS_TO_US(sleep_time_ms));
}

void sleep_by_timer_cb(){
    if (sleep_time_ms < 0) return;
    sleep_all();
}

esp_err_t create_sleep_timer(){
    const esp_timer_create_args_t sleep_enter_timer_args = {
        .callback = sleep_by_timer_cb,
        .name = "Enter sleep timer"
    };
    CHECK_ERROR(esp_timer_create(&sleep_enter_timer_args, &sleep_enter_timer), "Failed to create sleep timer");
    CHECK_ERROR(esp_timer_start_once(sleep_enter_timer, SECONDS_TO_US(sleep_time_ms)), "Failed to start sleep timer");
    return ESP_OK;
}

void touch_timer_reset(void * args){
    reset_sleep_enter_timer();
}


esp_err_t sleep_all_components(){
    CHECK_ERROR(display_sleep(), "Failed to put display to sleep");
    CHECK_ERROR(ble_sleep(), "Failed to put ble to sleep");
    return ESP_OK;
}
esp_err_t wake_up_all_components(){
    CHECK_ERROR(display_wakeup(), "Failed to wake up display");
    CHECK_ERROR(ble_wakeup(), "Failed to wake up ble");
    return ESP_OK;
}

esp_err_t sleep_all(){
    esp_err_t ret = sleep_all_components();
    esp_deep_sleep_start();
    return ret;
}

esp_err_t wake_up_all(){
    return wake_up_all_components();
}

esp_err_t sleep_init(void){
    CHECK_ERROR(esp_deep_sleep_enable_gpio_wakeup(BIT(BTN_GPIO), ESP_GPIO_WAKEUP_GPIO_HIGH), "Failed to enable gpio wakeup");
    CHECK_ERROR(esp_sleep_enable_timer_wakeup(MS_TO_US(DEEPSLEEP_PERIOD_MS)), "Failed to enable timer wakeup");
    if (sleep_time_ms > 0){
        touch_set_callback(touch_timer_reset, 0);
        create_sleep_timer();
    }
    return ESP_OK;
}
