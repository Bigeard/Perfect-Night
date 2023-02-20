#ifndef LINE_SEGMENT_H
#define LINE_SEGMENT_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "../../../lib/raylib/src/raylib.h"

    typedef struct LineSegment
    {
        Vector2 start;
        Vector2 end;
    } LineSegment;

    float Vector2CrossProduct(Vector2 a, Vector2 b);
    Vector2 Vector2SubtractCustom(Vector2 a, Vector2 b);
    bool CheckCollisionLineSegment(LineSegment a, LineSegment b);
    bool CheckCollisionPointSegment(float pointX, float pointY, LineSegment segment);
    bool CheckCollisionLineRec(LineSegment segment, Rectangle rec);
    bool CheckCollisionRecSegment(LineSegment segment, Rectangle rec);

#ifdef __cplusplus
}
#endif

#endif // LINE_SEGMENT_H
