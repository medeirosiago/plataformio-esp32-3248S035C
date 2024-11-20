#include <string.h>
#include "vars.h"

char air_temperature[100] = { 0 };

const char *get_var_air_temperature() {
    return air_temperature;
}

void set_var_air_temperature(const char *value) {
    strncpy(air_temperature, value, sizeof(air_temperature) / sizeof(char));
    air_temperature[sizeof(air_temperature) / sizeof(char) - 1] = 0;
}
