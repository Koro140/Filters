#version 330 core

in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D screenTexture;
float strength = 0.5;

void main()
{
    vec3 color = texture(screenTexture, TexCoord).rgb;

    float scanline = sin(TexCoord.y * 1200.0);

    color *= 1.0 - scanline * strength;

    FragColor = vec4(color, 1.0);
}