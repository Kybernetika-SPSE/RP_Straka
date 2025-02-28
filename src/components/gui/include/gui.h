#ifndef PT_GUI
#define PT_GUI

#include <src/display/lv_display.h>
#include "freertos/idf_additions.h"
#include "misc/lv_style.h"

typedef lv_obj_t* (*gui_screen_cb_t)(void);

// LVGL setup

#define PERIOD_MS 2
#define LVGL_TASK_STACK_SIZE (4 * 1024)
#define LVGL_TASK_PRIORITY 2
#define LVGL_TASK_MAX_DELAY 33

static SemaphoreHandle_t gui_mux = NULL;

// Screen layout

#define SCREENS_WIDTH 1
#define SCREENS_HEIGHT 1

#define DEFAULT_SCREEN_X 0
#define DEFAULT_SCREEN_Y 0

static uint8_t screen_x = DEFAULT_SCREEN_X;
static uint8_t screen_y = DEFAULT_SCREEN_Y;

static lv_obj_t* current_screen;

extern gui_screen_cb_t gui_screens[SCREENS_WIDTH][SCREENS_HEIGHT];

// Style setup

extern lv_style_t watchface_base;

void gui_screen_transition(uint8_t new_x, uint8_t new_y, lv_screen_load_anim_t anim);
void gui_init(void);
#endif
