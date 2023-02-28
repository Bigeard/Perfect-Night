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

void DrawTextureTiled(Texture2D texture, Rectangle source, Rectangle dest, Vector2 origin, float rotation, float scale, Color tint)
{
    if (texture.id <= 0 || scale <= 0.0f || source.width == 0 || source.height == 0 || dest.width < source.width * scale || dest.height < source.height * scale)
    {
        return;
    }

    int tileWidth = (int)(source.width * scale), tileHeight = (int)(source.height * scale);

    for (int dx = 0; dx < dest.width; dx += tileWidth)
    {
        int tileWidthClamped = min(dest.width - dx, tileWidth);
        float sourceWidthClamped = ((float)tileWidthClamped / tileWidth) * source.width;

        for (int dy = 0; dy < dest.height; dy += tileHeight)
        {
            int tileHeightClamped = min(dest.height - dy, tileHeight);
            float sourceHeightClamped = ((float)tileHeightClamped / tileHeight) * source.height;

            Rectangle destTile = (Rectangle){dest.x + dx, dest.y + dy, (float)tileWidthClamped, (float)tileHeightClamped};
            Rectangle sourceTile = (Rectangle){source.x, source.y, sourceWidthClamped, sourceHeightClamped};

            DrawTexturePro(texture, sourceTile, destTile, origin, rotation, tint);
        }
    }
}

int CalculateFontSizeWithMaxSize(const char* text, Vector2 maxSize, int margin)
{
    int fontSize = maxSize.x;
    while (maxSize.x - margin <= MeasureText(text, fontSize) || maxSize.y - margin <= fontSize)
    {
        fontSize--;
    }
    return fontSize;
}
