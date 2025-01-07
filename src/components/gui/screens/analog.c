#include "analog.h"
#include "core/lv_obj.h"
#include "widgets/label/lv_label.h"
#include "widgets/line/lv_line.h"
#include "gui.h"
#include <math.h>
#include <sys/time.h>

lv_obj_t* watchface_analog_init(void)
{
    //Background
    lv_obj_t* analog = lv_obj_create(NULL);
    lv_obj_set_size(analog, 466, 466);
    lv_obj_center(analog);
    lv_obj_remove_flag(analog, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(analog, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_style(analog, &watchface_base, 0);
    lv_obj_set_style_bg_color(analog, lv_color_hex(0x440000), LV_PART_MAIN);

    //Gray layer with all the watch stuff on it
    lv_obj_t * analog_mask2 = lv_obj_create(analog);
    lv_obj_set_size(analog_mask2, 466, 466);
    lv_obj_center(analog_mask2);
    lv_obj_add_style(analog_mask2, &watchface_base, 0);
    lv_obj_set_style_bg_opa(analog_mask2, LV_OPA_20, 0);

    //Minute line placement
    uint8_t i;
    for(i = 0; i < 60; i++) {
        lv_obj_t * line = lv_line_create(analog_mask2);

        //Every THICKER_LINE_FREQ-th line make a different shade
        uint8_t shade = i%THICKER_LINE_FREQ==0 ? 3 : 1;
        lv_obj_set_style_line_color(line, lv_palette_darken(LV_PALETTE_GREY, shade), 0);

        //Every THICKER_LINE_FREQ-th line make a different thickness
        uint8_t width = i%THICKER_LINE_FREQ==0 ? 3 : 2;
        lv_obj_set_style_line_width(line, width, 0);

        //Every LONG_LINE_FREQ-th and LONGER_LINE_FREQ-th line make a different length
        uint8_t inner_mod = i%LONG_LINE_FREQ==0 ? 7 : 0;
        inner_mod += i%LONGER_LINE_FREQ==0 ? 5 : 0;

        float angle = DEG6_IN_RAD*i;

        p[i][0].x=(int32_t)(OUTER_RADIUS*cos(angle))+DISPLAY_RADIUS;
        p[i][0].y=(int32_t)(OUTER_RADIUS*sin(angle))+DISPLAY_RADIUS;
        p[i][1].x=(int32_t)((INNER_RADIUS-inner_mod)*cos(angle))+DISPLAY_RADIUS;
        p[i][1].y=(int32_t)((INNER_RADIUS-inner_mod)*sin(angle))+DISPLAY_RADIUS;

        lv_line_set_points(line, p[i], 2);
    }

    //Number placement
    for(i = 1; i <= 12; i++) {
        lv_obj_t * number = lv_label_create(analog_mask2);
        lv_label_set_text_fmt(number, "%d", i);
        lv_obj_set_style_text_align(number, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_style_text_font(number, &lv_font_montserrat_48, 0);
        lv_obj_set_style_text_color(number, lv_palette_main(LV_PALETTE_GREY), 0);

        float angle = (DEG30_IN_RAD*i)-(3*DEG30_IN_RAD);
        int x = (int)((NUMBER_RADIUS)*cos(angle));
        int y = (int)((NUMBER_RADIUS)*sin(angle));
        lv_obj_align(number, LV_ALIGN_CENTER, x, y);
    }

    //Hour hand
    analog_hour = lv_line_create(analog_mask2);
    lv_obj_set_style_line_color(analog_hour, lv_palette_darken(LV_PALETTE_RED, 4), 0);
    lv_obj_set_style_line_width(analog_hour, 4, 0);
    lv_obj_set_style_line_rounded(analog_hour, true, 0);
    lv_line_set_points_mutable(analog_hour, hour_points, 2);
    lv_obj_set_size(analog_hour, 466, 466);

    //Minute hand
    analog_min = lv_line_create(analog_mask2);
    lv_obj_set_style_line_color(analog_min, lv_palette_darken(LV_PALETTE_GREY, 1), 0);
    lv_obj_set_style_line_width(analog_min, 3, 0);
    lv_obj_set_style_line_rounded(analog_min, true, 0);
    lv_line_set_points_mutable(analog_min, min_points, 2);
    lv_obj_set_size(analog_min, 466, 466);

    //Second hand
    analog_sec = lv_line_create(analog_mask2);
    lv_obj_set_style_line_color(analog_sec, lv_palette_darken(LV_PALETTE_ORANGE, 1), 0);
    lv_obj_set_style_line_width(analog_sec, 2, 0);
    lv_obj_set_style_line_rounded(analog_sec, true, 0);
    lv_line_set_points_mutable(analog_sec, sec_points, 2);
    lv_obj_set_size(analog_sec, 466, 466);

    //Axis - that small white circle in the middle
    lv_obj_t * analog_axis = lv_obj_create(analog_mask2);
    lv_obj_set_size(analog_axis, 15, 15);
    lv_obj_center(analog_axis);
    lv_obj_add_style(analog_axis, &watchface_base, 0);
    lv_obj_set_style_bg_color(analog_axis, lv_color_white(), 0);

    analog_timer = lv_timer_create(watchface_analog_update_time, 10, NULL);
    lv_obj_add_event_cb(analog, watchface_analog_remove_timer, LV_EVENT_DELETE, NULL);

    return analog;
}

void watchface_analog_update_time(lv_timer_t * timer)
{
    //Get current time - down to the microsecond
    struct timeval now;
    gettimeofday(&now, NULL);
    struct tm *tm_struct = localtime(&now.tv_sec);
    uint8_t time_hour = (tm_struct->tm_hour)%12;
    uint8_t time_min = (tm_struct->tm_min);
    uint8_t time_sec = (tm_struct->tm_sec);

    //Calculate hand angle
    #ifdef SMOOTH_HANDS
        float angle_hour = (DEG30_IN_RAD*time_hour)-(DEG90_IN_RAD)+(DEG6_IN_RAD*(time_min/12.));
        float angle_minute = (DEG6_IN_RAD*time_min)-(DEG90_IN_RAD)+(DEG6_IN_RAD*(time_sec/60.));
        float angle_sec = (DEG6_IN_RAD*time_sec)-(DEG90_IN_RAD)+(DEG6_IN_RAD*(now.tv_usec/1000000.));
    #else
        float angle_hour = (DEG30_IN_RAD*time_hour)-(DEG90_IN_RAD);
        float angle_minute = (DEG6_IN_RAD*time_min)-(DEG90_IN_RAD);
        float angle_sec = (DEG6_IN_RAD*time_sec)-(DEG90_IN_RAD);
    #endif

    //Calculate ending points of hound, minute and second hands
    hour_points[0].x = DISPLAY_RADIUS;
    hour_points[0].y = DISPLAY_RADIUS;
    hour_points[1].x = (int32_t)(INNER_RADIUS-80)*cos(angle_hour)+DISPLAY_RADIUS;
    hour_points[1].y = (int32_t)(INNER_RADIUS-80)*sin(angle_hour)+DISPLAY_RADIUS;

    min_points[0].x = DISPLAY_RADIUS;
    min_points[0].y = DISPLAY_RADIUS;
    min_points[1].x = (int32_t)(INNER_RADIUS-30)*cos(angle_minute)+DISPLAY_RADIUS;
    min_points[1].y = (int32_t)(INNER_RADIUS-30)*sin(angle_minute)+DISPLAY_RADIUS;

    sec_points[0].x = DISPLAY_RADIUS;
    sec_points[0].y = DISPLAY_RADIUS;
    sec_points[1].x = (int32_t)(INNER_RADIUS-40)*cos(angle_sec)+DISPLAY_RADIUS;
    sec_points[1].y = (int32_t)(INNER_RADIUS-40)*sin(angle_sec)+DISPLAY_RADIUS;

    //Refresh hands
    lv_obj_invalidate(analog_hour);
    lv_obj_invalidate(analog_min);
    lv_obj_invalidate(analog_sec);
}

void watchface_analog_remove_timer(lv_event_t * event){
    lv_timer_delete(analog_timer);
}
