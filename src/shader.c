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
