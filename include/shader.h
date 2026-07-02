#pragma once

#include <cglm/struct.h>

unsigned int shader_compile(const char *vertexSource, const char *fragmentSource, const char *geometrySource);
void shader_destroy(unsigned int *shader);

void shader_use(unsigned int shader);
void shader_set_float    (unsigned int shader, const char *name, float value);
void shader_set_integer  (unsigned int shader, const char *name, int value);
void shader_set_vector2f (unsigned int shader, const char *name, const vec2s value);
void shader_set_vector3f (unsigned int shader, const char *name, const vec3s value);
void shader_set_vector4f (unsigned int shader, const char *name, const vec4s value);
void shader_set_matrix4  (unsigned int shader, const char *name, const mat4s matrix);