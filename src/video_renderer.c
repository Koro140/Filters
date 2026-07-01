#include "video_renderer.h"

#include <stdio.h>
#include <glad/glad.h>
#include <SDL3/SDL.h>

#include "shader.h"
#include "texture.h"

int vao = 0;
int vbo = 0;

unsigned int y_tex = 0;
unsigned int u_tex = 0;
unsigned int v_tex = 0;

unsigned int get_y_tex() {
    return y_tex;
}

unsigned int get_u_tex() {
    return u_tex;
}

unsigned int get_v_tex() {
    return v_tex;
}

unsigned int shader_program = 0;

const float quad[] = {
 //Position        TexCoord
 -1.0f,-1.0f,      0.0f,1.0f,
  1.0f,-1.0f,      1.0f,1.0f,
  1.0f, 1.0f,      1.0f,0.0f,
 -1.0f, 1.0f,      0.0f,0.0f,
};

const char vetrex_src[] = 
                        "#version 330 core\n"
                        "layout(location = 0) in vec2 aPos;\n"
                        "layout(location = 1) in vec2 aTexCoord;\n"
                        
                        "out vec2 TexCoord;\n"

                        "void main()\n"
                        "{\n"
                        "    TexCoord = aTexCoord;\n"
                        "    gl_Position = vec4(aPos, 0.0, 1.0);\n"
                        "}\n";

const char fragment_src[] = 
                    "#version 330 core\n"
                    "in vec2 TexCoord;\n"

                    "out vec4 FragColor;\n"

                    "uniform sampler2D texY;\n"
                    "uniform sampler2D texU;\n"
                    "uniform sampler2D texV;\n"

                    "void main()\n"
                    "{\n"
                    "    float y = texture(texY, TexCoord).r;\n"

                    "    float u = texture(texU, TexCoord).r - 0.5;\n"
                    "    float v = texture(texV, TexCoord).r - 0.5;\n"

                    "    vec3 rgb;\n"

                    "    rgb.r = y + 1.402 * v;\n"
                    "    rgb.g = y - 0.344136 * u - 0.714136 * v;\n"
                    "    rgb.b = y + 1.772 * u;\n"

                    "    FragColor = vec4(rgb, 1.0);\n"
                    "}\n";

void video_renderer_init(int width, int height) {
    shader_program = shader_compile(vetrex_src, fragment_src, NULL);
    y_tex = texture_create_r8(width, height);
    u_tex = texture_create_r8(width / 2, height / 2);
    v_tex = texture_create_r8(width / 2, height / 2);

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1); 
    
    glBindVertexArray(0);
}

void video_renderer_destroy() {
    texture_destroy(&y_tex);
    texture_destroy(&u_tex);
    texture_destroy(&v_tex);

    shader_destroy(&shader_program);

    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
}

void video_renderer_draw(void)
{
    shader_use(shader_program);

    glUniform1i(glGetUniformLocation(shader_program, "texY"), 0);
    glUniform1i(glGetUniformLocation(shader_program, "texU"), 1);
    glUniform1i(glGetUniformLocation(shader_program, "texV"), 2);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, y_tex);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, u_tex);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, v_tex);

    glBindVertexArray(vao);

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    glBindVertexArray(0);
}