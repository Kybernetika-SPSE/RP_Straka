#include "board.h"
#include "button.h"
#include "driver/gpio.h"
#include "driver/i2c_types.h"
#include "esp_sleep.h"
#include "esp_log.h" // IWYU pragma: keep
#include "sleep.h"
#include "ioe_tca6408a.h"
#include "touch_chsc6x.h"
#include "pic_bq2562x.h"
#include "amoled_co5300.h"
#include "imu_icm42570.h"
#include "peripherals.h"
#include "ble_module.h"
#include "battery.h"

void ioe_gpio_set_direction(){
    ioe_set_reg_pin(ioe_reg_direction, 1, ioe_dir_output); //Set IOE pin 1 as output
}

void btn_callback(int event){
    static bool sleep = true;
    switch (event) {
        case BUTTON_STATE_CLICKED:
            if (sleep){
                sleep_all();
                sleep = false;
            }
            else {
                wake_up_all();
                sleep = true;
            }
        break;
        case BUTTON_STATE_HOLD:
            BQ2562x_setBATFETControl(BQ2562X_BATFET_CTRL_SHUTDOWN);
        break;
    }
}

void board_minimal_init(i2c_master_bus_handle_t * i2c_bus_handle){
    init_nvs();
    gpio_install_isr_service(0);
    board_init_i2c(i2c_bus_handle);
    board_init_spi();
    button_init(btn_callback);
    sleep_init();
}

void board_minimal_init_components(i2c_master_bus_handle_t i2c_bus_handle){
    display_minimal_init();
    ioe_init(i2c_bus_handle);
    BQ2562x_init(i2c_bus_handle);
    pic_create_periodic_timer(&meassure_battery);
    touch_init(i2c_bus_handle);
}

void board_full_init_components(i2c_master_bus_handle_t i2c_bus_handle){
    ioe_init(i2c_bus_handle);
    display_full_init();
    BQ2562x_init(i2c_bus_handle);
    pic_create_periodic_timer(&meassure_battery);
    pic_set_board_defaults();
    touch_init(i2c_bus_handle);
    ioe_init_interrupt(BOARD_INT2_IOE);
    imu_init(i2c_bus_handle);
    imu_setup_apex();
    imu_setup_debug_polling();
}

void board_init(void) {
    i2c_master_bus_handle_t i2c_bus_handle;
    board_minimal_init(&i2c_bus_handle);
    switch (esp_sleep_get_wakeup_cause()) {
        case ESP_SLEEP_WAKEUP_GPIO:
            board_minimal_init_components(i2c_bus_handle);
            wake_up_all();
            break;
        case ESP_SLEEP_WAKEUP_TIMER:
            BQ2562x_init(i2c_bus_handle);
            meassure_battery(); // yeah, its bad, it must wait while data isn't done then check and sleep
            esp_deep_sleep_start();
            break;
        default:
            board_full_init_components(i2c_bus_handle);
            ble_init();
    }
}
