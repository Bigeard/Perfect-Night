#include "../../../lib/raylib/src/raylib.h"


Color LightenColor(Color color, float percentage) {
    return (Color) {
        (255 - color.r) * (1 - percentage) + color.r, 
        (255 - color.g) * (1 - percentage) + color.g, 
        (255 - color.b) * (1 - percentage) + color.b, 
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