#include "sleep.h"
#include "amoled_co5300.h"
#include "button.h"
#include "driver/gpio.h"
#include "esp_sleep.h"
#include "hal/gpio_types.h"

RTC_DATA_ATTR int after_sleep = 0;

void RTC_IRAM_ATTR wake_stub_entry(void) {
    esp_default_wake_deep_sleep();
    after_sleep=1;
}

int get_after_sleep_state(){
    return after_sleep;
}

void sleep_all_components(){
    display_sleep();
}

void sleep_all(){
    sleep_all_components();
}

void wake_up_all(){
    display_wakeup();
}
