#ifndef WATCHFACE_ANALOG
#define WATCHFACE_ANALOG

#include "misc/lv_area.h"

extern lv_obj_t * analog;

#define DEG90_IN_RAD (90.0/180.0)*M_PI
#define DEG30_IN_RAD DEG90_IN_RAD/3
#define DEG6_IN_RAD DEG30_IN_RAD/5

#define DISPLAY_RADIUS 233

//Watchface radiuses
#define OUTER_RADIUS 231
#define INNER_RADIUS OUTER_RADIUS-5
#define NUMBER_RADIUS INNER_RADIUS-40

#define LONG_LINE_FREQ 5
#define LONGER_LINE_FREQ 15
#define THICKER_LINE_FREQ 15

static lv_point_precise_t p[60][2];
static lv_point_precise_t hour_points[2];
static lv_point_precise_t min_points[2];
static lv_point_precise_t sec_points[2];

static lv_obj_t * analog_hour;
static lv_obj_t * analog_min;
static lv_obj_t * analog_sec;

static lv_timer_t * analog_timer;

lv_obj_t* watchface_analog_init(void);
void watchface_analog_update_time(lv_timer_t * timer);
void watchface_analog_remove_timer(lv_event_t * event);
#endif
