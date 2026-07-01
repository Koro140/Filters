#pragma once

unsigned int shader_compile(const char *vertexSource, const char *fragmentSource, const char *geometrySource);
void shader_destroy(unsigned int *shader);

void shader_use(unsigned int shader);