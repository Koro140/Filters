#version 330 core

in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D screenTexture;

float strength = 0.3;   // 0.0 = normal, 0.3 = subtle, 0.7 = strong
float vignette = 0.7;   // 0.0 = none, 1.0 = strong

void main()
{
    // Center coordinates
    vec2 uv = TexCoord - 0.5;

    // Correct for screen aspect ratio
    float aspect = 16.0 / 9.0;
    uv.x *= aspect;

    float r = length(uv);

    // Smooth fisheye distortion
    float distortion = 1.0 + strength * r * r;

    vec2 distorted = uv * distortion;

    // Undo aspect correction
    distorted.x /= aspect;

    // Back to texture coordinates
    distorted += 0.5;

    // Smoothly fade edges instead of a harsh cutoff
    float edge = min(
        min(distorted.x, 1.0 - distorted.x),
        min(distorted.y, 1.0 - distorted.y)
    );

    float border = smoothstep(0.0, 0.04, edge);

    vec3 color = texture(screenTexture, distorted).rgb;

    // Subtle vignette
    float v = 1.0 - vignette * smoothstep(0.2, 0.75, r);

    color *= v;

    // Fade to black outside texture
    color *= border;

    FragColor = vec4(color, 1.0);
}