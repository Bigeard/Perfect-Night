#ifndef TOOL_H
#define TOOL_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "../../../lib/raylib/src/raylib.h"

    Color LightenColor(Color color, float percentage);
    Color DarkenColor(Color color, float percentage);
    Color ReverseColor(Color color, float percentage);

    float LerpCo(float start, float end, float t);
    Color LerpColor(Color colorA, Color colorB, float t);

    void DrawTextureTiled(const Texture2D texture, const Vector2 source, const Rectangle dest, const Color tint);

    int CalculateFontSizeWithMaxSize(const char *text, Vector2 maxSize, int margin);

#ifdef __cplusplus
}
#endif

#endif // TOOL_H
