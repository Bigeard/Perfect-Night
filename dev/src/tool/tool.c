#include "../../../lib/raylib/src/raylib.h"
#include "tool.h"


Color LightenColor(Color color, float percentage) {
    return (Color) {
        (255 - color.r) * (1 - percentage) + color.r, 
        (255 - color.g) * (1 - percentage) + color.g, 
        (255 - color.b) * (1 - percentage) + color.b, 
        color.a
    };
}

Color DarkenColor(Color color, float percentage) {
    return (Color) {
        color.r * percentage, 
        color.g * percentage, 
        color.b * percentage, 
        color.a
    };
}

Color ReverseColor(Color color, float percentage) {
    color.r = (255 - color.r - (1 - percentage * color.r)) + ((255 - color.r) * (1 - percentage) + color.r);
    color.g = (255 - color.g - (1 - percentage * color.g)) + ((255 - color.g) * (1 - percentage) + color.g);
    color.b = (255 - color.b - (1 - percentage * color.b)) + ((255 - color.b) * (1 - percentage) + color.b);

    return (Color) {
        color.r, 
        color.g, 
        color.b,
        color.a
    };
}

float LerpCo(float start, float end, float t) {
    return start + t*(end - start);
}

Color LerpColor(Color colorA, Color colorB, float t) {
    return (Color) {
        (unsigned char)LerpCo((float)colorA.r, (float)colorB.r, t),
        (unsigned char)LerpCo((float)colorA.g, (float)colorB.g, t),
        (unsigned char)LerpCo((float)colorA.b, (float)colorB.b, t),
        (unsigned char)LerpCo((float)colorA.a, (float)colorB.a, t),
    };
}
