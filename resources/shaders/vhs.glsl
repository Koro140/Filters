#version 330 core

in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D screenTexture;
uniform float time;       // pass elapsed time in seconds
uniform vec2 resolution;  // screen resolution

// Simple pseudo-random hash
float hash(vec2 p)
{
    return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453);
}

void main()
{
    vec2 uv = TexCoord;

    // --- Tracking distortion (horizontal jitter bands) ---
    float trackingNoise = hash(vec2(floor(uv.y * 40.0), time * 4.0));
    float tracking = (trackingNoise - 0.5) * 0.004;
    // Occasional bigger glitch band
    float glitchBand = step(0.995, hash(vec2(floor(uv.y * 10.0), floor(time * 6.0))));
    tracking += glitchBand * (hash(vec2(time)) - 0.5) * 0.05;
    uv.x += tracking;

    // --- Chromatic aberration (color channel split) ---
    float aberration = 0.004;
    float r = texture(screenTexture, uv + vec2(aberration, 0.0)).r;
    float g = texture(screenTexture, uv).g;
    float b = texture(screenTexture, uv - vec2(aberration, 0.0)).b;
    vec3 color = vec3(r, g, b);

    // --- Scanlines ---
    float scanline = sin(uv.y * resolution.y * 1.5) * 0.08;
    color -= scanline;

    // --- Static noise ---
    float noise = hash(uv * time) * 0.08;
    color += noise - 0.04;

    // --- Slight vertical roll / brightness flicker ---
    float flicker = 0.97 + 0.03 * sin(time * 10.0);
    color *= flicker;

    // --- Vignette ---
    vec2 center = uv - 0.5;
    float vignette = 1.0 - dot(center, center) * 0.6;
    color *= vignette;

    // --- Slight desaturation + contrast bump for that washed VHS look ---
    float gray = dot(color, vec3(0.299, 0.587, 0.114));
    color = mix(color, vec3(gray), 0.15);
    color = clamp((color - 0.5) * 1.1 + 0.5, 0.0, 1.0);

    FragColor = vec4(color, 1.0);
}