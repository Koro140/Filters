#version 330 core

in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D screenTexture;
uniform float time;
uniform float grainIntensity = 0.15;

float hash(vec2 p) {
    return fract(sin(dot(p, vec2(12.9898, 78.233))) * 43758.5453123);
}

void main() {
    vec3 color = texture(screenTexture, TexCoord).rgb;
    float grain = hash(TexCoord * time);
    color += (grain - 0.5) * grainIntensity;
    FragColor = vec4(color, 1.0);
}