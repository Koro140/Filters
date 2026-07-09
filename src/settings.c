#define FLAG_IMPLEMENTATION
#include "flag.h"

#define DYNAMIC_ARRAY_IMPLEMENTATION
#include "dynamic_array.h"

#include <stdio.h>

#include "settings.h"

typedef enum FilterType{
    FILTER_TYPE_NONE,
    FILTER_TYPE_VHS,
    FILTER_TYPE_BLACK_WHITE,
}FilterType;

typedef struct Settings {
    char* video_name;
    Dynamic_Array filter_types_array;
}Settings;

void usage(FILE *stream)
{
    fprintf(stream, "Usage: ./filters -V [VIDEO_NAME]\n");
    fprintf(stream, "OPTIONS:\n");
    flag_print_options(stream);
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
        if (strcmp(filter_flag_list.items[i], "none") == 0) {
            dynamic_array_push(&settings->filter_types_array, FILTER_TYPE_NONE);
        } else if (strcmp(filter_flag_list.items[i], "vhs") == 0) {
            dynamic_array_push(&settings->filter_types_array, FILTER_TYPE_VHS);
        } else if (strcmp(filter_flag_list.items[i], "bnw") == 0) {
            dynamic_array_push(&settings->filter_types_array, FILTER_TYPE_BLACK_WHITE);
        } else {
            fprintf(stderr, "Please provide a supported filter name after -F\n");
            fprintf(stderr, "Supported filters:\n");
            fprintf(stderr, "\tnone --- No filters\n");
            fprintf(stderr, "\tvhs  --- Old VHS effect\n");
            fprintf(stderr, "\tbnw  --- Black and white filter\n");
        }

    }
    
}

void settings_free(Settings* settings) {
    dynamic_array_free(&settings->filter_types_array);
}