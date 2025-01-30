#include <math.h>
#include <soc/gpio_num.h>
#include <stdbool.h>
#include <sys/_intsup.h>
#include "board.h"
#include "button.h"
#include "driver/gpio.h"
#include "driver/i2c_types.h"
#include "driver/spi_common.h"
#include "driver/i2c_master.h"
#include "ioe_tca6408a.h"
#include "sleep.h"
#include "touch_chsc6x.h"
#include "pic_bq2562x.h"
#include "amoled_co5300.h"
// #include "ble_module.h"
#include "esp_log.h"

static const char* TAG = "Board";

void board_init_spi(void) {
    spi_bus_config_t spi_bus_conf = {
        .sclk_io_num = DISPLAY_SPI_CLK,
        .data0_io_num = DISPLAY_SPI_DATA0,
        .data1_io_num = DISPLAY_SPI_DATA1,
        .data2_io_num = DISPLAY_SPI_DATA2,
        .data3_io_num = DISPLAY_SPI_DATA3,
        .max_transfer_sz = LCD_H_RES * LCD_V_RES * LCD_BIT_PER_PIXEL / 8,
    };
    ESP_ERROR_CHECK(spi_bus_initialize(DISPLAY_SPI_HOST, &spi_bus_conf, SPI_DMA_CH_AUTO));
}

i2c_master_bus_handle_t board_init_i2c(void) {
    i2c_master_bus_config_t i2c_bus_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = 0,
        .scl_io_num = SCL_GPIO,
        .sda_io_num = SDA_GPIO,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };
    i2c_master_bus_handle_t i2c_bus_handle;
    ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_bus_config, &i2c_bus_handle));
    return i2c_bus_handle;
}

void init_ioe_pins_direton(){
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
            BQ2562x_setBATFETControl(BQ2562X_BATFET_CTRL_SHIP_MODE);
        break;
    }
}

void pic_set_board_defaults(){
    BQ2562x_setChargeCurrent(160);
    BQ2562x_setIbatPk(BQ2562x_IBATPk_Limit1_5A);
    BQ2562x_setTopOff(BQ2562x_Timer17Min);
    BQ2562x_enableTS(false);
    BQ2562x_setupADC_defaults();
}

void board_init(void) {
    gpio_install_isr_service(0); // Needed to install gpio interrupts
    i2c_master_bus_handle_t i2c_bus_handle = board_init_i2c();

    board_init_spi();
    touch_init(i2c_bus_handle);
    BQ2562x_init(i2c_bus_handle);
    pic_set_board_defaults();
    button_init(btn_callback);
    ioe_init(i2c_bus_handle);
    init_CO5300();
    /// gooofy ahhh code zníčít co nejdříve
    ESP_LOGI(TAG, "Testing, getting PartInfo from BQ256x: %d", BQ2562x_getPartInformation());
    ESP_LOGI(TAG, "Testing, getting BATT V : %d", BQ2562x_getVBAT());
    ESP_LOGI(TAG, "Testing, getting BATT I : %d", BQ2562x_getIBAT());
    ESP_LOGI(TAG, "Testing, getting BUS I : %d", BQ2562x_getIBUS());
    ESP_LOGI(TAG, "Testing, getting Status charging: %d", BQ2562x_getChargeStat());
    ESP_LOGI(TAG, "wake up state: %i", get_after_sleep_state());
}
