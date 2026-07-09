#pragma once

#include <dynamic_array.h>

typedef enum FilterType{
    FILTER_TYPE_VHS,
    FILTER_TYPE_BLACK_WHITE,
}FilterType;

typedef struct Settings {
    char* video_name;
    Dynamic_Array filter_types_array;
}Settings;

void settings_get(Settings* settings, int argc, char** argv);
void settings_free(Settings* settings);