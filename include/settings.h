#pragma once

#include <dynamic_array.h>

typedef enum FilterType{
    FILTER_TYPE_VHS,
    FILTER_TYPE_BLACK_WHITE,
    FILTER_TYPE_BLUR,
    FILTER_TYPE_GLITCH,
    FILTER_TYPE_CHROMA,
    FILTER_TYPE_SCANLINE,
    FILTER_TYPE_FILM_GRAIN,
    FILTER_TYPE_VIGNETTE,
    FILTER_TYPE_PIXEL,
    FILTER_TYPE_EVIL
}FilterType;

typedef struct Settings {
    char* video_name;
    Dynamic_Array filter_types_array;
    char* export_name;
    bool export_mode;
}Settings;

void settings_get(Settings* settings, int argc, char** argv);
void settings_free(Settings* settings);