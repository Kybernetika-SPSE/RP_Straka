#ifndef SLEEP_H
#define SLEEP_H

#include "esp_err.h"

#define DEEPSLEEP_PERIOD_MS CONFIG_DEEPSLEEP_PERIOD_MS
#define DEFAULT_SLEEP_TIMER_PERIOD_SECONDS CONFIG_DEFAULT_SLEEP_TIMER_VALUE

esp_err_t sleep_init(void);
esp_err_t sleep_all();
esp_err_t wake_up_all();

#endif
