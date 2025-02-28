#include <src/display/lv_display.h>
#include "core/lv_obj_event.h"
#include "draw/lv_draw_rect.h"
#include "misc/lv_timer.h"
#include "esp_timer.h"
#include "gui.h"

// Screens
#include "analog.h"
#include "digital.h"

lv_style_t watchface_base;

gui_screen_cb_t gui_screens[SCREENS_WIDTH][SCREENS_HEIGHT];

void gui_increase_lvgl_tick() {
    /* Tell LVGL how many milliseconds has elapsed */
    lv_tick_inc(PERIOD_MS);
}

bool gui_mux_lock(int timeout_ms) {
    assert(gui_mux);

    const TickType_t timeout_ticks = (timeout_ms == -1) ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms);
    return xSemaphoreTake(gui_mux, timeout_ticks) == pdTRUE;
}

void gui_mux_unlock(void) {
    assert(gui_mux);
    xSemaphoreGive(gui_mux);
}

void gui_task() {
    uint32_t task_delay_ms = LVGL_TASK_MAX_DELAY;
    while (1) {
        // Lock the mutex due to the LVGL APIs not being thread-safe
        if (gui_mux_lock(-1)) {
            task_delay_ms = lv_timer_handler();
            // Release the mutex
            gui_mux_unlock();
        }
        // Clamp delay
        if (task_delay_ms > LVGL_TASK_MAX_DELAY) {
            task_delay_ms = LVGL_TASK_MAX_DELAY;
        } else if (task_delay_ms < LVGL_TASK_MAX_DELAY) {
            task_delay_ms = LVGL_TASK_MAX_DELAY;
        }
        vTaskDelay(pdMS_TO_TICKS(task_delay_ms));
    }
}

void gui_swipe_event(lv_event_t * e)
{
    //lv_obj_t * screen = lv_event_get_current_target(e);
    lv_screen_load_anim_t anim;
    uint8_t new_x = screen_x;
    uint8_t new_y = screen_y;
    lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_active());
    switch(dir) {
        // TODO: FIX UNDER AND OVERFLOW
        case LV_DIR_LEFT:
            new_x++;
            anim = LV_SCR_LOAD_ANIM_OVER_LEFT;
            break;
        case LV_DIR_RIGHT:
            new_x--;
            anim = LV_SCR_LOAD_ANIM_OVER_RIGHT;
            break;
        case LV_DIR_TOP:
            new_y++;
            anim = LV_SCR_LOAD_ANIM_OVER_TOP;
            break;
        case LV_DIR_BOTTOM:
            new_y--;
            anim = LV_SCR_LOAD_ANIM_OVER_BOTTOM;
            break;
        default:
            anim = LV_SCR_LOAD_ANIM_NONE;
            break;
    }
    gui_screen_transition(new_x, new_y, anim);
}

void gui_screen_transition(uint8_t new_x, uint8_t new_y, lv_screen_load_anim_t anim){
    if (new_x <= (SCREENS_WIDTH-1) && new_y <= (SCREENS_HEIGHT-1)){
        if (gui_screens[new_x][new_y] != NULL){
            current_screen = gui_screens[new_x][new_y]();
            lv_obj_add_event_cb(current_screen, gui_swipe_event, LV_EVENT_GESTURE, NULL);
            lv_screen_load_anim(current_screen, anim, 200, 0, true);
            screen_x = new_x;
            screen_y = new_y;
        }
    }
}

void gui_screen_jump(void){
    gui_screen_transition(DEFAULT_SCREEN_X, DEFAULT_SCREEN_Y, LV_SCR_LOAD_ANIM_FADE_ON);
}

void gui_init(void) {
    // Tick interface for LVGL (using esp_timer to generate 2ms periodic event)
    const esp_timer_create_args_t lvgl_tick_timer_args = {
        .callback = &gui_increase_lvgl_tick,
        .name = "lvgl_tick"
    };
    esp_timer_handle_t lvgl_tick_timer = NULL;
    ESP_ERROR_CHECK(esp_timer_create(&lvgl_tick_timer_args, &lvgl_tick_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(lvgl_tick_timer, PERIOD_MS * 1000));

    gui_mux = xSemaphoreCreateMutex();
    assert(gui_mux);
    xTaskCreate(gui_task, "LVGL", LVGL_TASK_STACK_SIZE, NULL, LVGL_TASK_PRIORITY, NULL);

    lv_style_init(&watchface_base);
    lv_style_set_bg_color(&watchface_base, lv_color_hex(0x000000));
    lv_style_set_border_width(&watchface_base, 0);
    lv_style_set_border_side(&watchface_base, LV_BORDER_SIDE_NONE);
    lv_style_set_radius(&watchface_base, LV_RADIUS_CIRCLE);
    lv_style_set_pad_all(&watchface_base, 0);
    /*Make a gradient*/
    lv_style_set_bg_opa(&watchface_base, LV_OPA_COVER);
    static lv_grad_dsc_t grad;
    grad.dir = LV_GRAD_DIR_VER;
    grad.stops_count = 2;
    grad.stops[0].color = lv_color_hex(0x2e0c4d);
    grad.stops[0].opa = LV_OPA_COVER;
    grad.stops[1].color = lv_color_hex(0x0c134d);
    grad.stops[1].opa = LV_OPA_COVER;

    /*Shift the gradient to the bottom*/
    grad.stops[0].frac  = 0;
    grad.stops[1].frac  = 255;

    lv_style_set_bg_grad(&watchface_base, &grad);

    // Add screen init callbacks to array

    // gui_screens[1][0] = watchface_analog_init;
    gui_screens[0][0] = watchface_digital_init;
}
