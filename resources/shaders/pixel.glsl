#version 330 core

in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D screenTexture;
uniform vec2 resolution;
uniform float pixelSize = 8.0;

void main() {
    vec2 grid = resolution / pixelSize;
    vec2 snappedUV = floor(TexCoord * grid) / grid;
    FragColor = vec4(texture(screenTexture, snappedUV).rgb, 1.0);
}