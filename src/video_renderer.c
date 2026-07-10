#include "video_renderer.h"

#include <stdio.h>
#include <glad/glad.h>

#include "shader.h"
#include "texture.h"

typedef struct Render_Target {
    unsigned int texture;
    unsigned int framebuffer;

    int width;
    int height;
}Render_Target;

SDL_Window* window_reference = NULL;

int vao = 0;
int vbo = 0;

unsigned int y_tex = 0;
unsigned int u_tex = 0;
unsigned int v_tex = 0;

unsigned int yuv_to_rgb_shader = 0;
unsigned int to_screen_shader = 0;

unsigned int* filters_shaders = NULL;
int filter_shaders_count = 0;

Render_Target rt1 = {0};
Render_Target rt2 = {0};

const float quad[] = {
 //Position        TexCoord
 -1.0f,-1.0f,      0.0f,0.0f,
  1.0f,-1.0f,      1.0f,0.0f,
  1.0f, 1.0f,      1.0f,1.0f,
 -1.0f, 1.0f,      0.0f,1.0f,
};

// exter globals from filter.c
extern const char* filter_vertex_src;
extern const char* to_screen_vertex_src;
extern const char* yuv_to_rgb_src;

// filters
extern const char* to_screen_src;
extern const char* black_white_src;
extern const char* vhs_src;
extern const char* blur_src;

void render_target_create(Render_Target* rt,int width, int height) {
    rt->width = width;
	rt->height = height;

	glCreateFramebuffers(1, &rt->framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, rt->framebuffer);

	glGenTextures(1, &rt->texture);
	glBindTexture(GL_TEXTURE_2D, rt->texture);
  
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);  

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, rt->texture, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		printf("ERROR::FRAME_BUFFER::Couldn't complete the frame buffer creation\n");
	}
}

void render_target_free(Render_Target* rt) {
	glDeleteTextures(1, &rt->texture);
	glDeleteFramebuffers(1, &rt->framebuffer);
}

void video_renderer_init(SDL_Window* window ,Settings* settings, int width, int height) {
    window_reference = window;
    
    yuv_to_rgb_shader = shader_compile(filter_vertex_src, yuv_to_rgb_src, NULL);
    to_screen_shader = shader_compile(to_screen_vertex_src, to_screen_src, NULL);

    filters_shaders = malloc(sizeof(unsigned int) * settings->filter_types_array.count);
    if (filters_shaders == NULL) {
        fprintf(stderr, "Couldn't allocate memory for filters shaders list\n");
        exit(1);
    }
    filter_shaders_count = settings->filter_types_array.count;

    for (int i = 0; i < settings->filter_types_array.count; i++) {
        switch (((FilterType*)settings->filter_types_array.items)[i]) {
        case FILTER_TYPE_VHS:
            filters_shaders[i] = shader_compile(filter_vertex_src, vhs_src, NULL);
            break;    
        case FILTER_TYPE_BLACK_WHITE:
            filters_shaders[i] = shader_compile(filter_vertex_src, black_white_src, NULL);
            break;
        case FILTER_TYPE_BLUR:
            filters_shaders[i] = shader_compile(filter_vertex_src, blur_src, NULL);
            break;
        }
    }
    
    y_tex = texture_create_r8(width, height);
    u_tex = texture_create_r8(width / 2, height / 2);
    v_tex = texture_create_r8(width / 2, height / 2);

    render_target_create(&rt1, width, height);
    render_target_create(&rt2, width, height);

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
    render_target_free(&rt1);
    render_target_free(&rt2);

    texture_destroy(&y_tex);
    texture_destroy(&u_tex);
    texture_destroy(&v_tex);

    shader_destroy(&yuv_to_rgb_shader);
    shader_destroy(&to_screen_shader);

    for (int i = 0; i < filter_shaders_count; i++) {
        shader_destroy(&filters_shaders[i]);
    }
    

    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
}

void video_renderer_draw(AVFrame* frame)
{
    glBindFramebuffer(GL_FRAMEBUFFER, rt1.framebuffer);
        // Upload texture
        glViewport(0,0, rt1.width, rt1.height);
        upload_texture_r8(y_tex, frame->data[0], frame->width, frame->height, frame->linesize[0]);
        upload_texture_r8(u_tex, frame->data[1], frame->width / 2, frame->height / 2, frame->linesize[1]);
        upload_texture_r8(v_tex, frame->data[2], frame->width / 2, frame->height / 2, frame->linesize[2]);
        
        // Clear background
        glClearColor(0.0, 0.0, 0.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);
        
        // Draw the texture using the shader
        shader_use(yuv_to_rgb_shader);
        
        shader_set_integer(yuv_to_rgb_shader, "texY", 0);
        shader_set_integer(yuv_to_rgb_shader, "texU", 1);
        shader_set_integer(yuv_to_rgb_shader, "texV", 2);
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, y_tex);
        
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, u_tex);
        
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, v_tex);
        
        glBindVertexArray(vao);
        
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        
        glBindVertexArray(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    bool rt1_as_input = true;
    vec2s resolution = {rt1.width, rt1.height};
    for (int i = 0; i < filter_shaders_count; i++) {
        glBindFramebuffer(GL_FRAMEBUFFER, rt1_as_input ? rt2.framebuffer : rt1.framebuffer);
        glViewport(0,0, rt1.width, rt1.height);
        shader_use(filters_shaders[i]);
        shader_set_integer(filters_shaders[i], "screenTexture", 0);
        shader_set_float(filters_shaders[i], "time",  SDL_GetTicksNS() / (float)SDL_NS_PER_SECOND);
        shader_set_vector2f(filters_shaders[i], "resolution", resolution);
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, rt1_as_input ? rt1.texture : rt2.texture);

        glClearColor(0.0, 0.0, 0.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

        glBindVertexArray(0);
        rt1_as_input = !rt1_as_input;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    int w, h;
    SDL_GetWindowSize(window_reference, &w, &h);

    float texAspect = (float)rt1.width / rt2.height;
    float winAspect  = (float)w / h;

    float scaleX = 1.0f, scaleY = 1.0f;
    if (winAspect > texAspect) {
        scaleX = texAspect / winAspect;
    } else {
        scaleY = winAspect / texAspect;
    }
    glViewport(0, 0, w, h);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, rt1_as_input ? rt1.texture : rt2.texture);
    shader_use(to_screen_shader);
    shader_set_integer(to_screen_shader, "screenTexture", 0);
    shader_set_vector2f(to_screen_shader, "scale", (vec2s){scaleX, scaleY});

    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glBindVertexArray(0);
}