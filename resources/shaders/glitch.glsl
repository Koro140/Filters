#version 330 core

in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D screenTexture;
uniform float time;

float rand(vec2 p)
{
    return fract(sin(dot(p, vec2(12.9898, 78.233))) * 43758.5453);
}

void main()
{
    vec2 uv = TexCoord;

    // Random horizontal glitch lines
    float line = floor(uv.y * 100.0);

    if (rand(vec2(line, floor(time * 15.0))) > 0.95)
    {
        uv.x += (rand(vec2(line, time)) - 0.5) * 0.1;
    }

    // Random blocks
    vec2 block = floor(uv * vec2(16.0, 10.0));

    if (rand(block + floor(time * 10.0)) > 0.97)
    {
        uv.x += (rand(block) - 0.5) * 0.15;
    }

    // Get the image
    vec3 color = texture(screenTexture, uv).rgb;

    // Noise
    color += (rand(uv * time * 300.0) - 0.5) * 0.04;

    // Random flash
    if (rand(vec2(floor(time * 8.0), 1.0)) > 0.98)
    {
        color *= 1.3;
    }

    FragColor = vec4(color, 1.0);
}