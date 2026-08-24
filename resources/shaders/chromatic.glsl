#version 330 core

in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D screenTexture;

const vec2 OFFSET = vec2(0.005, 0.0);

void main() {
    float r = texture(screenTexture, TexCoord - OFFSET).r;
    float g = texture(screenTexture, TexCoord).g;
    float b = texture(screenTexture, TexCoord + OFFSET).b;

    FragColor = vec4(r, g, b, 1.0);
}