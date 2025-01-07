#ifndef WATCHFACE_DIGITAL
#define WATCHFACE_DIGITAL

#include "misc/lv_types.h"

extern lv_obj_t * digital;

static lv_obj_t * label_clock;
static lv_obj_t * label_date;

static lv_timer_t * digital_timer;

static char time_string[9];
static char date_string[13];

lv_obj_t* watchface_digital_init(void);
void watchface_digital_tick(lv_timer_t * timer);
void watchface_digital_update_time(void);
void watchface_digital_remove_timer(lv_event_t * event);
#endif
