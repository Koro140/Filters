#version 330 core

in vec2 v_texCoord;
out vec4 fragColor;

uniform sampler2D u_texture;

const vec2 OFFSET = vec2(0.005, 0.0);

void main() {
    float r = texture(u_texture, v_texCoord - OFFSET).r;
    float g = texture(u_texture, v_texCoord).g;
    float b = texture(u_texture, v_texCoord + OFFSET).b;

    fragColor = vec4(r, g, b, 1.0);
}