#define FLAG_IMPLEMENTATION
#include "flag.h"

#define DYNAMIC_ARRAY_IMPLEMENTATION
#include "dynamic_array.h"

#include <stdio.h>

#include "settings.h"

void supported_filters(FILE* stream) {
    fprintf(stream, "Please provide a supported filter name after -F\n");
    fprintf(stream, "Supported filters:\n");
    fprintf(stream, "\tvhs    --- Old VHS effect\n");
    fprintf(stream, "\tbnw    --- Black and white filter\n");
    fprintf(stream, "\tblur   --- Blurry effect\n");
    fprintf(stream, "\tglitch --- Glitchy effect\n");
    fprintf(stream, "\tchroma --- Chromatic Abbaration effect\n");
    fprintf(stream, "\tscanline --- Old CRT TV lines effect\n");
}

void usage(FILE *stream)
{
    fprintf(stream, "Usage: ./filters -V [VIDEO_NAME]\n");
    fprintf(stream, "Usage: ./filters -V [VIDEO_NAME] -F [FILTER] -F [FILTER] .....\n");
    fprintf(stream, "OPTIONS:\n");
    flag_print_options(stream);

    supported_filters(stream);
}

void settings_get(Settings* settings, int argc, char** argv) {
    bool help = false;
    Flag_List filter_flag_list = {0};
    settings->filter_types_array = dynamic_arr_init(sizeof(FilterType), 8);

    flag_str_var(&settings->video_name, "V", "", "Video name");
    flag_bool_var(&help, "help", false, "How to use this app");
    flag_list_var(&filter_flag_list, "F", "Filters list to apply in sequential order");

    if (!flag_parse(argc, argv)) {
        usage(stderr);
        flag_print_error(stderr);
        exit(1);
    }

    argc = flag_rest_argc();
    argv = flag_rest_argv();

    // checking for the help flag
    if (help) {
        usage(stdout);
        exit(0);
    }

    // checking if video name exists
    if (strlen(settings->video_name) == 0) {
        fprintf(stderr,"Please provide a video name\n");
        fprintf(stderr, "Usage: ./filters -V [VIDEO_NAME]\n");
        fprintf(stderr, "For more info use ./filters -help\n");
        exit(1);
    }
    
    // Doing the filters type parsing
    for (size_t i = 0; i < filter_flag_list.count; i++) {
        FilterType t;
        if (strcmp(filter_flag_list.items[i], "vhs") == 0) {
            t = FILTER_TYPE_VHS;
        } else if (strcmp(filter_flag_list.items[i], "bnw") == 0) {
            t = FILTER_TYPE_BLACK_WHITE;
        } else if (strcmp(filter_flag_list.items[i], "blur") == 0) {
            t = FILTER_TYPE_BLUR;
        } else if (strcmp(filter_flag_list.items[i], "glitch") == 0) {
            t = FILTER_TYPE_GLITCH;
        } else if (strcmp(filter_flag_list.items[i], "chroma") == 0) {
            t = FILTER_TYPE_CHROMA;
        } else if (strcmp(filter_flag_list.items[i], "scanline") == 0) {
            t = FILTER_TYPE_SCANLINE;
        } else {
            supported_filters(stderr);
            settings_free(settings);
            exit(1);
        }

        dynamic_array_append(&settings->filter_types_array, &t);
    }
}

void settings_free(Settings* settings) {
    dynamic_array_free(&settings->filter_types_array);
}