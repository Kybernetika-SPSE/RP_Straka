#include "display/lv_display.h"
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include "amoled_co5300.h"
#include "board.h"
#include "indev/lv_indev.h"
#include "lv_init.h"
#include "lvgl.h" // IWYU pragma: keep
#include "touch_chsc6x.h"
#include "lv_init.h"
#include "gui.h"

// static const char *TAG = "PlajTime";

void rounder_event_cb(lv_event_t * e)
{
    lv_area_t * a = lv_event_get_invalidated_area(e);
    a->x1 = a->x1 & (~0x1); /* Ensure x1 is even */
    a->x2 = a->x2 | 0x1;    /* Ensure x2 is odd */
}

void app_main() {
    board_init();
    lv_init();
    static uint8_t buf1[LVGL_BUFFER];
    static uint8_t buf2[LVGL_BUFFER];
    // DISPLAY
    lv_display_t * display = lv_display_create(466, 466);
    lv_display_set_buffers(display, buf1, buf2, LVGL_BUFFER, LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_antialiasing(display, false);
    lv_display_set_flush_cb(display, display_flush_cb);
    lv_display_add_event_cb(display, rounder_event_cb, LV_EVENT_INVALIDATE_AREA, NULL);
    // TOUCH
    lv_indev_t * indev = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev, touch_lvgl_cb);

    gui_init();

    gui_screen_transition(DEFAULT_SCREEN_X+1, DEFAULT_SCREEN_Y, LV_SCR_LOAD_ANIM_NONE);
}
