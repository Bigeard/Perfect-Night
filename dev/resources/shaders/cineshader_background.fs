#version 100

precision mediump float;

uniform float iTime;
uniform vec2 iResolution;

float rand(vec2 n)
{
    return fract(sin(dot(n, vec2(12.9898, 4.1414))) * 43758.5453);
}

float noise(vec2 p)
{
    vec2 ip = floor(p);
    vec2 u = fract(p);
    u = u*u*(3.0 - 2.0*u);

    float res = mix(
        mix(rand(ip), rand(ip + vec2(1.0, 0.0)), u.x),
        mix(rand(ip + vec2(0.0, 1.0)), rand(ip + vec2(1.0, 1.0)), u.x),
        u.y);
    return res*res;
}

const mat2 m2 = mat2(0.8, -0.6, 0.6, 0.8);

float fbm(vec2 p)
{
    float f = 0.0;
    f += 0.5000*noise(p); p = m2*p*2.02;
    f += 0.2500*noise(p); p = m2*p*2.03;
    f += 0.1250*noise(p); p = m2*p*2.01;
    f += 0.0625*noise(p);

    return f/0.769;
}

float pattern(vec2 p)
{
    vec2 q = vec2(fbm(p));
    vec2 r = vec2(fbm(p + 4.0*q + vec2(1.7, 9.2)));
    r += iTime*0.15;
    return fbm(p + 1.760*r);
}

void main()
{
    const float pixelSize = 6.0;
    vec2 pixelCoord = floor(gl_FragCoord.xy/pixelSize)*pixelSize + pixelSize*0.5;
    vec2 uv = pixelCoord/iResolution.xy;
    uv *= 4.5;

    float displacement = pattern(uv);
    vec3 purple = vec3(0.498, 0.400, 0.949); // #7f66f2
    vec3 cyan = vec3(0.251, 0.902, 0.902);   // #40e6e6
    vec3 color = mix(purple, cyan, clamp(displacement, 0.0, 1.0));

    // Preserve the supplied CineShader depth over Perfect Night's #212530 base.
    float alpha = min(displacement*0.30, 1.0);
    gl_FragColor = vec4(color, alpha);
}
