idf_component_register(SRCS "amoled_co5300.c"
                    INCLUDE_DIRS "include"
                    REQUIRES esp_driver_spi
                    PRIV_REQUIRES ioe_tca6408a board
                    )
