#include "ble_module.h"
#include "esp_err.h"
#include "esp_log.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "host/ble_hs.h"
#include "nimble/nimble_port.h"
#include "utils_module.h"
#include "gap.h"
#include "gatt_svc.h"

static const char* TAG = "BLE-MAIN";

static void set_nible_config(void) {
    ble_hs_cfg.sync_cb = adv_init;
    ble_hs_cfg.store_status_cb = ble_store_util_status_rr;
}

static void nimble_host_task(void *param) {
    ESP_LOGI(TAG, "nimble host task has been started!");
    nimble_port_run();
    vTaskDelete(NULL);
}

esp_err_t ble_init(void) {
    CHECK_ERROR(nimble_port_init(), "failed to initialize nimble stack");
    CHECK_ERROR(gap_init(), "failed to initialize nimble stack");
    CHECK_ERROR(gatt_svc_init(), "failed to initialize GATT server");
    set_nible_config();
    xTaskCreate(nimble_host_task, "NimBLE Host", 4*1024, NULL, 5, NULL);
    return ESP_OK;
}

esp_err_t ble_sleep(void){
    CHECK_ERROR(ble_gap_adv_stop(), "failed to stop advertising")
    CHECK_ERROR(nimble_port_stop(), "failed to stop NimBLE");
    CHECK_ERROR(nimble_port_deinit(), "failed to deinit NimBLE");
    return ESP_OK;
}

esp_err_t ble_wakeup(void){
    return ble_init();
}
