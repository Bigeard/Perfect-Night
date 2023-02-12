#ifndef TOOL_H
#define TOOL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../../../lib/raylib/src/raylib.h"

Color LightenColor(Color color, float percentage);
Color DarkenColor(Color color, float percentage);
Color ReverseColor(Color color, float percentage);

float LerpCo(float start, float end, float t);
Color LerpColor(Color colorA, Color colorB, float t);

#ifdef __cplusplus
}
#endif

#endif // TOOL_H