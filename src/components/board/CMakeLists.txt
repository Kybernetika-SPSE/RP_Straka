idf_component_register(SRCS "board.c" "button.c" "sleep.c" "peripherals.c" "battery.c"
                    INCLUDE_DIRS "include"
                    REQUIRES esp_driver_spi esp_driver_i2c
                    )
