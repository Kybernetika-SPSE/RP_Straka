#ifndef GAP_SVC_H
#define GAP_SVC_H

#include "esp_err.h"
#define BLE_GAP_APPEARANCE_GENERIC_TAG 0x00C0
#define BLE_GAP_URI_PREFIX_HTTPS 0x17
#define BLE_GAP_LE_ROLE_PERIPHERAL 0x00

void adv_init(void);
esp_err_t gap_init(void);

#endif
