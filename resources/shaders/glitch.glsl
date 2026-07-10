#version 330 core

in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D screenTexture;
uniform float time;

float rand(vec2 p)
{
    return fract(sin(dot(p, vec2(12.9898,78.233))) * 43758.5453);
}

void main()
{
    vec2 uv = TexCoord;

    //-----------------------------
    // Random horizontal slices
    //-----------------------------
    float line = floor(uv.y * 140.0);

    if(rand(vec2(line, floor(time * 18.0))) > 0.93)
    {
        uv.x += (rand(vec2(line, time)) - 0.5) * 0.12;
    }

    //-----------------------------
    // Block corruption
    //-----------------------------
    vec2 block = floor(uv * vec2(20.0, 12.0));

    if(rand(block + floor(time * 12.0)) > 0.96)
    {
        uv.x += (rand(block) - 0.5) * 0.18;
        uv.y += (rand(block + 3.1) - 0.5) * 0.04;
    }

    //-----------------------------
    // RGB Split
    //-----------------------------
    float shift = 0.006;

    vec2 rUV = uv + vec2( shift, 0.0);
    vec2 gUV = uv;
    vec2 bUV = uv - vec2( shift, 0.0);

    float r = texture(screenTexture, rUV).r;
    float g = texture(screenTexture, gUV).g;
    float b = texture(screenTexture, bUV).b;

    vec3 color = vec3(r, g, b);

    //-----------------------------
    // Scanlines
    //-----------------------------
    float scan = sin(uv.y * 900.0) * 0.03;
    color -= scan;

    //-----------------------------
    // White Noise
    //-----------------------------
    color += (rand(uv * time * 300.0) - 0.5) * 0.05;

    //-----------------------------
    // Random Flash
    //-----------------------------
    if(rand(vec2(floor(time * 8.0), 1.0)) > 0.98)
    {
        color *= 1.4;
    }

    FragColor = vec4(color, 1.0);
}