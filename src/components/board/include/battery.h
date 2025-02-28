#ifndef BATTERY_H
#define BATTERY_H

#include "esp_timer.h"

#define BATTERY_ASK_PERIOD_MS 5000
#define TS_INSTALLED false

void pic_set_board_defaults();
void meassure_battery();
void pic_create_periodic_timer(esp_timer_cb_t battery_meassure_init);

static uint16_t vbus;
static int16_t ibus;
static uint16_t vbat;
static int16_t ibat;

static bool bat_dead;

#endif
