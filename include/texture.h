#pragma once

#include <inttypes.h>

unsigned int texture_create_r8(int width, int height);
void texture_destroy(unsigned int* texture);
void upload_texture_r8(unsigned int texture, const uint8_t *data, int width, int height, int padding);