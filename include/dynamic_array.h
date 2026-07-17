#pragma once

typedef struct Dynamic_Array {
    int count;
    int capacity;
    void* items;
    int item_size;
}Dynamic_Array;

Dynamic_Array dynamic_arr_init(int item_size, int initial_capacity);
void dynamic_array_append(Dynamic_Array* arr, void* item);
void dynamic_array_free(Dynamic_Array* arr);

#ifdef DYNAMIC_ARRAY_IMPLEMENTATION

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

Dynamic_Array dynamic_arr_init(int item_size, int initial_capacity) {
    Dynamic_Array arr = {0};
    if (initial_capacity <= 0) {
        initial_capacity = 8;
    }
    
    void* alloced_mem = malloc(item_size * initial_capacity);    
    if (alloced_mem == NULL) {
        fprintf(stderr, "ERROR::Dynamic_Array::Couldn't create a dynaimc array\n");
    } else {
        arr.item_size = item_size;
        arr.items = alloced_mem;
        arr.capacity = initial_capacity;
        arr.count = 0;
    }
    return arr;
}

void dynamic_array_append(Dynamic_Array* arr, void* item) {
    // reallocation routine
    if (arr == NULL) {
        fprintf(stderr, "ERROR::Dynamic_Array::Couldn't append to uninitialize array\n");
        return;
    }
    
    if (arr->capacity == arr->count) {
        size_t new_capacity = arr->capacity * 2;
        void* new_meme = realloc(arr->items, new_capacity * arr->item_size);
        if (new_meme == NULL) {
            fprintf(stderr, "ERROR::Dynamic_Array::Couldn't allocate more memory for dynamic array\n");
            return;
        }

        arr->items = new_meme;
        arr->capacity = new_capacity;
    }

    memcpy(((char*)arr->items) + arr->count * arr->item_size, item, arr->item_size);
    arr->count++;
}

void dynamic_array_free(Dynamic_Array* arr) {
    free(arr->items);
    arr->items = NULL;
    arr->capacity = 0;
    arr->item_size = 0;
    arr->count = 0;
}

#endif