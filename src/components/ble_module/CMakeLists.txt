idf_component_register(SRCS "ble_module.c" "gap.c" "gatt_svc.c"
                    INCLUDE_DIRS "include"
                    REQUIRES nvs_flash bt
)
