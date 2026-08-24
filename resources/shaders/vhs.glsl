#version 330 core

in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D screenTexture;
uniform float time;
uniform vec2 resolution;

float hash(vec2 p)
{
    return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453);
}

void main()
{
    vec2 uv = TexCoord;

    // Subtle horizontal tracking distortion
    float line = floor(uv.y * 50.0);
    float noise = hash(vec2(line, floor(time * 8.0)));

    uv.x += (noise - 0.5) * 0.003;

    // Occasional small tracking glitch
    float glitch = step(0.995, hash(vec2(floor(time * 5.0), line)));

    uv.x += glitch * (hash(vec2(line, time)) - 0.5) * 0.025;

    // Original image — no chromatic aberration
    vec3 color = texture(screenTexture, uv).rgb;

    // Subtle scanlines
    float scanline = sin(uv.y * resolution.y * 1.5);
    color *= 1.0 - scanline * 0.025;

    // Fine VHS noise
    float grain = hash(uv * resolution + time * 20.0);
    color += (grain - 0.5) * 0.025;

    // Slight brightness flicker
    color *= 0.99 + 0.01 * sin(time * 8.0);

    // Very subtle vignette
    vec2 center = uv - 0.5;
    float vignette = 1.0 - dot(center, center) * 0.25;
    color *= vignette;

    FragColor = vec4(color, 1.0);
}