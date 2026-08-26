#version 330 core

in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D screenTexture;
uniform float vignetteStrength = 1;
uniform float vignetteRadius = 0.75;

void main() {
    vec3 color = texture(screenTexture, TexCoord).rgb;
    float dist = length(TexCoord - vec2(0.5));
    float vig = smoothstep(vignetteRadius, vignetteRadius - 0.4, dist);
    color *= mix(1.0 - vignetteStrength, 1.0, vig);
    FragColor = vec4(color, 1.0);
}