#pragma once

#include <stdlib.h>

typedef struct dynamic_string {
    char* data;
    int count;
    int capacity;
} dynamic_string;

dynamic_string dynamic_string_create(int capacity);
dynamic_string dynamic_string_append(dynamic_string dist, const char* src);
dynamic_string dynamic_string_free(dynamic_string s);
dynamic_string dynamic_string_from_file(const char* filePath);

#ifdef DYNAMIC_STRING_IMPLEMENTATION
    dynamic_string dynamic_string_create(int capacity) {
        dynamic_string s;
        s.data = capacity > 0 ? malloc(capacity * sizeof(char)) : NULL;

        if (s.data == NULL) {
            fprintf(stderr, "ERROR::Couldn't create dynamic_string of capacity %d\n", capacity);
            return (dynamic_string){ .data = NULL, .capacity = 0, .count = 0 };
        }

        s.capacity = capacity;
        s.count = 0;
        return s;
    }

    dynamic_string dynamic_string_append(dynamic_string dist, const char* src) {
        int len = strlen(src);
        while (len + dist.count + 1 > dist.capacity) {
            int newCap = 0;
            if (dist.capacity < 8) {
                newCap = 8;
            } else {
                newCap = dist.capacity * 2;
            }

            char* newData = realloc(dist.data, newCap);
            if (newData == NULL) {
                fprintf(stderr, "ERROR::Couldn't allocate more memory for string\n");
                exit(1);
            }

            dist.capacity = newCap;
            dist.data = newData;
        }

        memcpy(dist.data + dist.count, src, len);

        dist.count = dist.count + len;
        dist.data[dist.count] = '\0';

        return dist;
    }

    dynamic_string dynamic_string_free(dynamic_string s) {
        free(s.data);
        return (dynamic_string){
            .data = NULL,
            .capacity = 0,
            .count = 0,
        };
    }

    dynamic_string dynamic_string_from_file(const char* filePath) {
        FILE* file = fopen(filePath, "rb");

        if (file == NULL) {
            fprintf(stderr, "ERROR::Couldn't open file %s\n", filePath);
            return (dynamic_string){0};
        }

        fseek(file, 0, SEEK_END);
        long size = ftell(file);
        rewind(file);

        dynamic_string s = {
            .data = malloc(size + 1),
            .count = size,
            .capacity = size + 1
        };

        if (s.data == NULL) {
            fprintf(stderr, "ERROR::Couldn't allocate memory for reading file\n");
            return (dynamic_string){0};
        }

        fread(s.data, 1, size, file);
        s.data[size] = '\0';

        fclose(file);

        return s;
    }
#endif