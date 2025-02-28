#include "digital.h"
#include "core/lv_obj.h"
#include "gui.h"
#include "widgets/label/lv_label.h"
#include <sys/time.h>

lv_obj_t* watchface_digital_init(void)
{
    lv_obj_t* digital = lv_obj_create(NULL);
    lv_obj_set_size(digital, 466, 466);
    lv_obj_center(digital);
    lv_obj_remove_flag(digital, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(digital, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_style(digital, &watchface_base, 0);
    lv_obj_set_style_bg_color(digital, lv_color_hex(0x440000), LV_PART_MAIN);

    lv_obj_t* digital_mask2 = lv_obj_create(digital);
    lv_obj_set_size(digital_mask2, 466, 466);
    lv_obj_center(digital_mask2);
    lv_obj_remove_flag(digital_mask2, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_style(digital_mask2, &watchface_base, 0);
    lv_obj_set_style_bg_opa(digital_mask2, LV_OPA_20, 0);

    // create label
    label_clock = lv_label_create(digital);
    label_date = lv_label_create(digital);

    watchface_digital_update_time();

    lv_label_set_text_static(label_clock, time_string);
    lv_obj_align(label_clock, LV_ALIGN_CENTER, 0, -10);

    lv_label_set_text_static(label_date, date_string);
    lv_obj_align(label_date, LV_ALIGN_CENTER, 0, 24);

    lv_obj_set_style_text_font(label_clock, &lv_font_montserrat_48, 0);
    lv_obj_set_style_text_color(label_clock, lv_palette_main(LV_PALETTE_GREY), 0);
    lv_obj_set_style_text_font(label_date, &lv_font_montserrat_26, 0);
    lv_obj_set_style_text_color(label_date, lv_palette_main(LV_PALETTE_GREY), 0);

    digital_timer = lv_timer_create(watchface_digital_tick, 100, NULL);
    lv_obj_add_event_cb(digital, watchface_digital_remove_timer, LV_EVENT_DELETE, NULL);

    return digital;
}

void watchface_digital_tick(lv_timer_t * timer){
    watchface_digital_update_time();
}

void watchface_digital_update_time(void)
{
    //Get current time - down to the microsecond
    struct timeval now;
    gettimeofday(&now, NULL);
    struct tm *tm_struct = localtime(&now.tv_sec);
    uint8_t time_hour = (tm_struct->tm_hour);
    uint8_t time_min = (tm_struct->tm_min);
    uint8_t time_sec = (tm_struct->tm_sec);

    // The actual year is returned from 1900 but since we do % 100 we don't need to add that
    // Otherwise the full year would be ((tm_struct->tm_year) + 1900)
    int year = (tm_struct->tm_year) % 100;
    uint8_t mon = (tm_struct->tm_mon) + 1;
    uint8_t day = tm_struct->tm_mday;

    sprintf(time_string, "%02d:%02d:%02d", time_hour&0x1F, time_min&0x3F, time_sec&0x3F);

    sprintf(date_string, "%02d. %02d. %02d", day&0x1F, mon&0xF, year);

    lv_obj_invalidate(label_clock);
    lv_obj_invalidate(label_date);
}

void watchface_digital_remove_timer(lv_event_t * event){
    lv_timer_delete(digital_timer);
}
