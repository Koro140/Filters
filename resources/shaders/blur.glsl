#version 330 core

in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D screenTexture;
uniform vec2 resolution;

void main()
{
    vec2 texelSize = 1.0 / resolution;
    vec3 result = vec3(0.0);
    float total = 0.0;

    // 5x5 Gaussian-ish kernel
    const int radius = 6;
    for (int x = -radius; x <= radius; ++x) {
        for (int y = -radius; y <= radius; ++y) {
            float weight = 1.0 - (length(vec2(x, y)) / (float(radius) * 1.5));
            weight = max(weight, 0.0);

            vec2 offset = vec2(float(x), float(y)) * texelSize;
            result += texture(screenTexture, TexCoord + offset).rgb * weight;
            total += weight;
        }
    }

    result /= total;

    FragColor = vec4(result, 1.0);
}