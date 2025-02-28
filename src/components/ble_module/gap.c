#include "gap.h"
#include "esp_err.h"
#include "ble_module.h"
#include "host/util/util.h"
#include "services/gap/ble_svc_gap.h"
#include "host/ble_gap.h"
#include "utils_module.h"
#include <stdint.h>

static const char* TAG = "BLE-GAP";

inline static void format_addr(char *addr_str, uint8_t addr[]);
static void print_conn_desc(struct ble_gap_conn_desc *desc);
static int gap_event_handler(struct ble_gap_event *event, void *arg);

static uint8_t own_addr_type;
static uint8_t addr_val[6] = {0};
static const uint8_t esp_uri[] = {BLE_GAP_URI_PREFIX_HTTPS, '/', '/', 'p', 'l', 'a', 'j', 't', 'a', '.', 'e', 'u'};
inline static void format_addr(char *addr_str, uint8_t addr[]) {
    sprintf(addr_str, "%02X:%02X:%02X:%02X:%02X:%02X", addr[0], addr[1],
            addr[2], addr[3], addr[4], addr[5]);
}

static void print_conn_desc(struct ble_gap_conn_desc *desc) {
    /* Local variables */
    char addr_str[18] = {0};

    /* Connection handle */
    ESP_LOGI(TAG, "connection handle: %d", desc->conn_handle);

    /* Local ID address */
    format_addr(addr_str, desc->our_id_addr.val);
    ESP_LOGI(TAG, "device id address: type=%d, value=%s",
             desc->our_id_addr.type, addr_str);

    /* Peer ID address */
    format_addr(addr_str, desc->peer_id_addr.val);
    ESP_LOGI(TAG, "peer id address: type=%d, value=%s", desc->peer_id_addr.type,
             addr_str);

    /* Connection info */
    ESP_LOGI(TAG,
             "conn_itvl=%d, conn_latency=%d, supervision_timeout=%d, "
             "encrypted=%d, authenticated=%d, bonded=%d\n",
             desc->conn_itvl, desc->conn_latency, desc->supervision_timeout,
             desc->sec_state.encrypted, desc->sec_state.authenticated,
             desc->sec_state.bonded);
}

static esp_err_t start_advertising(void) {
    const char *name = ble_svc_gap_device_name();
    struct ble_hs_adv_fields adv_fields = {
        .flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP,
        .name = (uint8_t *)name,
        .name_len = strlen(name),
        .name_is_complete = 1,
        .tx_pwr_lvl = BLE_HS_ADV_TX_PWR_LVL_AUTO,
        .tx_pwr_lvl_is_present = 1,
        .appearance = BLE_GAP_APPEARANCE_GENERIC_TAG,
        .appearance_is_present = 1,
        .le_role = BLE_GAP_LE_ROLE_PERIPHERAL,
        .le_role_is_present = 1,
    };
    CHECK_ERROR(ble_gap_adv_set_fields(&adv_fields), "failed to set advertising data")
    struct ble_hs_adv_fields rsp_fields = {
        .device_addr = addr_val,
        .device_addr_type = own_addr_type,
        .device_addr_is_present = 1,
        .uri = esp_uri,
        .uri_len = sizeof(esp_uri),
        .adv_itvl = BLE_GAP_ADV_ITVL_MS(500),
        .adv_itvl_is_present = 1,
    };
    CHECK_ERROR(ble_gap_adv_rsp_set_fields(&rsp_fields), "failed to set scan response data");
    struct ble_gap_adv_params adv_params = {
        .conn_mode = BLE_GAP_CONN_MODE_UND,
        .disc_mode = BLE_GAP_DISC_MODE_GEN,
        .itvl_min = BLE_GAP_ADV_ITVL_MS(500),
        .itvl_max = BLE_GAP_ADV_ITVL_MS(510),
    };
    CHECK_ERROR(
        ble_gap_adv_start(own_addr_type, NULL, BLE_HS_FOREVER, &adv_params, gap_event_handler, NULL),
        "failed to start advertising"
    );
    ESP_LOGI(TAG, "advertising started!");
    return ESP_OK;
}


static int gap_event_handler(struct ble_gap_event *event, void *arg) {
    struct ble_gap_conn_desc desc;
    switch (event->type) {
    case BLE_GAP_EVENT_CONNECT:
        ESP_LOGI(TAG, "connection %s; status=%d",
                 event->connect.status == 0 ? "established" : "failed",
                 event->connect.status);
        if (event->connect.status == 0) { // Conected
            CHECK_ERROR(ble_gap_conn_find(event->connect.conn_handle, &desc), "failed to find connection by handle");
            print_conn_desc(&desc);
            struct ble_gap_upd_params params = {
                .itvl_min = desc.conn_itvl,
                .itvl_max = desc.conn_itvl,
                .latency = 3,
                .supervision_timeout = desc.supervision_timeout
            };
            CHECK_ERROR(ble_gap_update_params(event->connect.conn_handle, &params), "failed to update connection parameters");
        }
        else { // Not Connected
            start_advertising();
        }
        break;

    case BLE_GAP_EVENT_DISCONNECT:
        ESP_LOGI(TAG, "disconnected from peer; reason=%d", event->disconnect.reason);
        start_advertising();
        break;

    case BLE_GAP_EVENT_ADV_COMPLETE:
        ESP_LOGI(TAG, "advertise complete; reason=%d", event->adv_complete.reason);
        start_advertising();
        break;
    }
    return ESP_OK;
}

void adv_init(void) {
    char addr_str[18] = {0};
    ESP_ERROR_CHECK(ble_hs_util_ensure_addr(0));
    /* Figure out BT address to use while advertising (no privacy for now) */
    ESP_ERROR_CHECK(ble_hs_id_infer_auto(0, &own_addr_type));
    ESP_ERROR_CHECK(ble_hs_id_copy_addr(own_addr_type, addr_val, NULL));
    format_addr(addr_str, addr_val);
    ESP_LOGI(TAG, "device address: %s", addr_str);
    start_advertising();
}

esp_err_t gap_init(void) {
    esp_err_t ret = 0;
    ble_svc_gap_init();
    ret = ble_svc_gap_device_name_set(DEVICE_NAME);
    CHECK_ERROR(ret, "failed to set device name");
    return ret;
}
