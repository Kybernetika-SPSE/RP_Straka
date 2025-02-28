#include "host/ble_hs.h"
#include "services/gatt/ble_svc_gatt.h"
#include "utils_module.h"
#include "gatt_svc.h"
#include <sys/time.h>
#include "host/ble_gatt.h"

static const char* TAG = "BLE-GATT";

static const ble_uuid16_t settings_service_uuid = BLE_UUID16_INIT(0x1805);
static uint16_t date_characteristic_handle;
static const ble_uuid16_t date_characteristic_uuid = BLE_UUID16_INIT(0x2A2B);

static int date_characteristic_access_cb(uint16_t conn_handle, uint16_t attr_handle,
                                      struct ble_gatt_access_ctxt *ctxt, void *arg) {
    int rc = 0;
    if (ctxt->op == BLE_GATT_ACCESS_OP_WRITE_CHR) {
        if (conn_handle != BLE_HS_CONN_HANDLE_NONE) {
            ESP_LOGI(TAG, "characteristic write; conn_handle=%d attr_handle=%d", conn_handle, attr_handle);
        } else {
            ESP_LOGI(TAG, "characteristic write by nimble stack; attr_handle=%d", attr_handle);
        }
        if (attr_handle == date_characteristic_handle) {
            if (ctxt->om->om_len == 10) {
                uint8_t *data = ctxt->om->om_data;
                struct tm timeinfo;
                timeinfo.tm_year = ((data[1] << 8) | data[0]) - 1900;
                timeinfo.tm_mon = data[2] - 1;
                timeinfo.tm_mday = data[3];
                timeinfo.tm_hour = data[4];
                timeinfo.tm_min = data[5];
                timeinfo.tm_sec = data[6];
                timeinfo.tm_wday = data[7] % 7;
                struct timeval tv = {
                    .tv_sec = mktime(&timeinfo),
                    .tv_usec = SECONDS_FRACTIONS_TO_US(data[8])
                };
                ESP_LOGI(TAG, "Recived Time: Year: %i, Mon: %i, day: %i, Hour: %i, Min: %i, Sec: %i, WDay: %i, Usec: %li",
                    timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_wday, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec, timeinfo.tm_wday, tv.tv_usec);
                settimeofday(&tv, NULL);
                return rc;
            }
        }
    }
    ESP_LOGE(TAG, "Invalid operation or attribute handle");
    return -1;
}

static const struct ble_gatt_svc_def gatt_service_table[] = {
    {.type = BLE_GATT_SVC_TYPE_PRIMARY,
     .uuid = &settings_service_uuid.u,
     .characteristics =
        (struct ble_gatt_chr_def[]){
            {.uuid = &date_characteristic_uuid.u,
                .access_cb = date_characteristic_access_cb,
                .flags = BLE_GATT_CHR_F_WRITE,
                .val_handle = &date_characteristic_handle},
            {0}},
    },
    {
        0, /* No more services. */
    },
};

int gatt_svc_init(void) {
    ble_svc_gatt_init();
    CHECK_ERROR(ble_gatts_count_cfg(gatt_service_table), "Failed to adjust host configuration")
    CHECK_ERROR(ble_gatts_add_svcs(gatt_service_table), "Failed to queue service definitions")
    return ESP_OK;
}
