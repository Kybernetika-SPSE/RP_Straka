#ifndef BUTTON_H
#define BUTTON_H

#include "esp_err.h"
#define BTN_GPIO GPIO_NUM_3
#define BUTTON_TASK_STACK_SIZE 2048
#define BUTTON_COOLDOWN_MS 300
#define BUTTON_HOLD_MS 500

enum {
    BUTTON_STATE_CLICKED,
    BUTTON_STATE_HOLD
};

typedef void (*button_callback_t)(int);

esp_err_t button_init(button_callback_t callback);

#endif
