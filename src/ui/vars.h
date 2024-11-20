#ifndef EEZ_LVGL_UI_VARS_H
#define EEZ_LVGL_UI_VARS_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// enum declarations



// Flow global variables

enum FlowGlobalVariables {
    FLOW_GLOBAL_VARIABLE_AIR_TEMPERATURE = 0,
    FLOW_GLOBAL_VARIABLE_ROOM_TEMPERATURE = 1
};

// Native global variables

extern const char *get_var_air_temperature();
extern void set_var_air_temperature(const char *value);
extern const char *get_var_room_temperature();
extern void set_var_room_temperature(const char *value);


#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_VARS_H*/