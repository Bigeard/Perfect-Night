#include "../../../lib/raylib/src/raylib.h"
#include "../../../lib/raylib/src/raymath.h"
#include "tool.h"

Color LightenColor(Color color, float percentage)
{
    return (Color){
        (255.0f - color.r) * (1.0f - percentage) + color.r,
        (255.0f - color.g) * (1.0f - percentage) + color.g,
        (255.0f - color.b) * (1.0f - percentage) + color.b,
        color.a};
}

Color DarkenColor(Color color, float percentage)
{
    return (Color){
        color.r * percentage,
        color.g * percentage,
        color.b * percentage,
        color.a};
}

Color ReverseColor(Color color, float percentage)
{
    color.r = (255.0f - color.r - (1.0f - percentage * color.r)) + ((255.0f - color.r) * (1.0f - percentage) + color.r);
    color.g = (255.0f - color.g - (1.0f - percentage * color.g)) + ((255.0f - color.g) * (1.0f - percentage) + color.g);
    color.b = (255.0f - color.b - (1.0f - percentage * color.b)) + ((255.0f - color.b) * (1.0f - percentage) + color.b);

    return (Color){
        color.r,
        color.g,
        color.b,
        color.a};
}

float LerpCo(float start, float end, float t)
{
    return start + t * (end - start);
}

Color LerpColor(Color colorA, Color colorB, float t)
{
    return (Color){
        (unsigned char)LerpCo((float)colorA.r, (float)colorB.r, t),
        (unsigned char)LerpCo((float)colorA.g, (float)colorB.g, t),
        (unsigned char)LerpCo((float)colorA.b, (float)colorB.b, t),
        (unsigned char)LerpCo((float)colorA.a, (float)colorB.a, t),
    };
}

int min(int a, int b)
{
    if (a < b)
    {
        return a;
    }
    else
    {
        return b;
    }
}

void DrawTextureTiled(const Texture2D texture, const Vector2 source, const Rectangle dest, const Color tint)
{
    for (float dx = 0; dx < dest.width; dx += source.x)
    {
        const int tileWidthClamped = min(dest.width - dx, source.x);
        for (float dy = 0; dy < dest.height; dy += source.y)
        {
            const int tileHeightClamped = min(dest.height - dy, source.y);
            const Rectangle s = {0, 0, tileWidthClamped, tileHeightClamped};
            const Vector2 p = {dest.x + dx, dest.y + dy};
            DrawTextureRec(texture, s, p, tint);
        }
    }
}

int CalculateFontSizeWithMaxSize(const char *text, Vector2 maxSize, int margin)
{
    int fontSize = maxSize.x;
    while (maxSize.x - margin <= MeasureText(text, fontSize) || maxSize.y - margin <= fontSize)
    {
        fontSize--;
    }
    return fontSize;
}
