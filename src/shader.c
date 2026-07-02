#include "shader.h"

#include <glad/glad.h>
#include <stdio.h>

unsigned int shader_compile(const char* vertexSource, const char* fragmentSource, const char* geometrySource) {
    int success;
    char log[512];
    
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSource, NULL);
    glCompileShader(vertexShader);
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, log);
        fprintf(stderr, "ERROR::VERTEX_SHADER::%s\n", log);
    }

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, log);
        fprintf(stderr, "ERROR::FRAGMENT_SHADER::%s\n", log);
    }

    unsigned int geometryShader = 0;
    if (geometrySource != NULL) {
        geometryShader = glCreateShader(GL_GEOMETRY_SHADER);
        glShaderSource(geometryShader, 1, &geometrySource, NULL);
        glCompileShader(geometryShader);
        glGetShaderiv(geometryShader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(geometryShader, 512, NULL, log);
            fprintf(stderr, "ERROR::GEOMETRY_SHADER::%s\n", log);
        }
    }

    unsigned int programID = glCreateProgram();
    glAttachShader(programID, vertexShader);
    glAttachShader(programID, fragmentShader);
    if (geometrySource != NULL)
        glAttachShader(programID, geometryShader);

    glLinkProgram(programID);
    glGetProgramiv(programID, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(programID, 512, NULL, log);
        fprintf(stderr, "ERROR::SHADER_LINK::%s\n", log);
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    if (geometrySource != NULL)
        glDeleteShader(geometryShader);

    return programID;
}

void shader_destroy(unsigned int *shader) {
    glDeleteProgram(*shader);
    *shader = 0;
}

void shader_use(unsigned int shader)
{
    glUseProgram(shader);
}

void shader_set_float(unsigned int shader, const char *name, float value)
{
    glUniform1f(glGetUniformLocation(shader, name),value);
}

void shader_set_integer(unsigned int shader, const char *name, int value)
{
    glUniform1i(glGetUniformLocation(shader, name), value);
}

void shader_set_vector2f(unsigned int shader, const char *name, const vec2s value)
{
    glUniform2f(glGetUniformLocation(shader, name), value.x, value.y);
}

void shader_set_vector3f(unsigned int shader, const char *name, const vec3s value)
{
    glUniform3f(glGetUniformLocation(shader, name), value.x, value.y, value.z);
}

void shader_set_vector4f(unsigned int shader, const char *name, const vec4s value)
{
    glUniform4f(glGetUniformLocation(shader, name), value.x, value.y, value.z, value.w);
}

void shader_set_matrix4(unsigned int shader, const char *name, const mat4s matrix)
{
    glUniformMatrix4fv(glGetUniformLocation(shader, name), 1, GL_FALSE, (const float *)matrix.raw);
}