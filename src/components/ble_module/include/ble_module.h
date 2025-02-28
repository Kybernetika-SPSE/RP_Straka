#ifndef BLE_MODULE_H
#define BLE_MODULE_H

#include "esp_err.h"
#define DEVICE_NAME "PlajTime"

esp_err_t ble_init(void);
esp_err_t ble_sleep(void);
esp_err_t ble_wakeup(void);

#endif
