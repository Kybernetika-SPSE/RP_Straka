#ifndef SLEEP_H
#define SLEEP_H

#include "esp_err.h"

#define DEEPSLEEP_PERIOD_MS 10000
#define DEFAULT_SLEEP_TIMER_PERIOD_SECONDS 30

esp_err_t sleep_init(void);
esp_err_t sleep_all();
esp_err_t wake_up_all();

#endif
