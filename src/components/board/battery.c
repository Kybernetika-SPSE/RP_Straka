#include "battery.h"
#include "esp_log.h"
#include "pic_bq2562x.h"
#include "utils_module.h"
#include "esp_timer.h"
#include <stdbool.h>

static const char* TAG = "Battery";

void pic_set_board_defaults(){
    bat_dead = false;
    BQ2562x_setChargeCurrentLimit(160);
    BQ2562x_setIbatPk(BQ2562x_IBATPk_Limit1_5A);
    BQ2562x_setTopOff(BQ2562x_Timer17Min);
    BQ2562x_enableTS(TS_INSTALLED);
    BQ2562x_setupADC_oneshot();
}

void meassure_battery(){
    vbus = BQ2562x_getVBUS();
    ibus = BQ2562x_getIBUS();
    vbat = BQ2562x_getVBAT();
    ibat = BQ2562x_getIBAT();
    BQ2562x_VBUSStat bus_stat = BQ2562x_getVBUSStat();
    BQ2562x_ChargeStat charge_stat = BQ2562x_getChargeStat();
    uint8_t fault_stat = BQ2562x_getFaultStatus();
    bool ts_en = BQ2562x_getTSEnabled();

    ESP_LOGI(TAG, "Testing, getting BATT V : %d", vbat);
    ESP_LOGI(TAG, "Testing, getting BATT I : %d", ibat);
    ESP_LOGI(TAG, "Testing, getting BUS V : %d", vbus);
    ESP_LOGI(TAG, "Testing, getting BUS I : %d", ibus);
    ESP_LOGI(TAG, "Testing, getting VBUS status: %d", bus_stat);
    ESP_LOGI(TAG, "Testing, getting Status charging: %d", charge_stat);
    ESP_LOGI(TAG, "Testing, getting FAULT status: %d", fault_stat);
    ESP_LOGI(TAG, "Testing, getting FAULT flag: %d", BQ2562x_getFaultFlag());
    ESP_LOGI(TAG, "Testing, getting TS status: %d", ts_en);
    ESP_LOGI(TAG, "Testing, getting TS bias: %f", BQ2562x_getTSBias());
    ESP_LOGI(TAG, "Testing, getting current limit: %d", BQ2562x_getChargeCurrentLimit());
    ESP_LOGI(TAG, "Testing, is it even set to charge? %d", BQ2562x_getChargingEnabled());

    if (ts_en != TS_INSTALLED) {
        BQ2562x_enableTS(TS_INSTALLED);  // TODO: Actually fix this, instead of bruteforcing it...
    }

    if ((charge_stat == BQ2562X_CHARGE_STAT_TERMINATED) && (vbus < 4000) && (vbat < 3200)) {
        if (bat_dead) {
            ESP_LOGI(TAG, "U FUCKER!!! U KILLED ME BY LETTING ME DISCHARGE TOO MUCH!!!");
            BQ2562x_setBATFETControl(BQ2562X_BATFET_CTRL_SHUTDOWN);
        }
        else {
            bat_dead = true;
        }
    } else {
        bat_dead = false;
    }

    BQ2562x_setupADC_oneshot();
}

void pic_create_periodic_timer(esp_timer_cb_t battery_meassure_init){
    const esp_timer_create_args_t battery_adc_timer_args = {
        .callback = battery_meassure_init,
        .name = "battery_meassure"
    };
    esp_timer_handle_t battery_adc_timer = NULL;
    ESP_ERROR_CHECK(esp_timer_create(&battery_adc_timer_args, &battery_adc_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(battery_adc_timer, MS_TO_US(BATTERY_ASK_PERIOD_MS)));
}
