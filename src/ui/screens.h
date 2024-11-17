#ifndef EEZ_LVGL_UI_SCREENS_H
#define EEZ_LVGL_UI_SCREENS_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _objects_t {
    lv_obj_t *main;
    lv_obj_t *air_temperature;
    lv_obj_t *screen00;
    lv_obj_t *obj0;
    lv_obj_t *obj1;
    lv_obj_t *button_logout;
} objects_t;

extern objects_t objects;

enum ScreensEnum {
    SCREEN_ID_MAIN = 1,
    SCREEN_ID_AIR_TEMPERATURE = 2,
    SCREEN_ID_SCREEN00 = 3,
};

void create_screen_main();
void tick_screen_main();

void create_screen_air_temperature();
void tick_screen_air_temperature();

void create_screen_screen00();
void tick_screen_screen00();

void create_screens();
void tick_screen(int screen_index);


#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_SCREENS_H*/