#include "../../../lib/raylib/src/raylib.h"
#include "../../../lib/raylib/src/raymath.h"

#include "line_segment.h"

// Function to compute the cross product of two vectors
float Vector2CrossProduct(Vector2 a, Vector2 b)
{
    return a.x * b.y - a.y * b.x;
}

// Function to subtract two vectors
Vector2 Vector2SubtractCustom(Vector2 a, Vector2 b)
{
    Vector2 result = {a.x - b.x, a.y - b.y};
    return result;
}

// Function to check if two line segments intersect
bool CheckCollisionLineSegment(LineSegment a, LineSegment b)
{
    Vector2 p = a.start;
    Vector2 r = Vector2SubtractCustom(a.end, a.start);
    Vector2 q = b.start;
    Vector2 s = Vector2SubtractCustom(b.end, b.start);

    float t = Vector2CrossProduct(Vector2SubtractCustom(q, p), s) / Vector2CrossProduct(r, s);
    float u = Vector2CrossProduct(Vector2SubtractCustom(q, p), r) / Vector2CrossProduct(r, s);

    // Check if the line segments intersect
    if (t >= 0 && t <= 1 && u >= 0 && u <= 1)
    {
        return true;
    }

    return false;
}

bool CheckCollisionPointSegment(float pointX, float pointY, LineSegment segment)
{
    // Check if the point is inside the bounding box of the line segment
    if (pointX < fmin(segment.start.x, segment.end.x) || pointX > fmax(segment.start.x, segment.end.x) ||
        pointY < fmin(segment.start.y, segment.end.y) || pointY > fmax(segment.start.y, segment.end.y))
    {
        return false;
    }

    // Check if the point is on the line segment
    float dist = (segment.end.y - segment.start.y) * pointX + (segment.start.x - segment.end.x) * pointY +
                 (segment.end.x * segment.start.y - segment.start.x * segment.end.y);
    if (fabs(dist) > 0.000001f)
    {
        return false;
    }

    return true;
}

bool CheckCollisionLineRec(LineSegment segment, Rectangle rec)
{
    // Create a line segment for each side of the rectangle
    LineSegment side1 = {{rec.x, rec.y}, {rec.x + rec.width, rec.y}};
    LineSegment side2 = {{rec.x + rec.width, rec.y}, {rec.x + rec.width, rec.y + rec.height}};
    LineSegment side3 = {{rec.x + rec.width, rec.y + rec.height}, {rec.x, rec.y + rec.height}};
    LineSegment side4 = {{rec.x, rec.y + rec.height}, {rec.x, rec.y}};

    // Check if the line segment intersects any of the sides of the rectangle
    if (CheckCollisionLineSegment(segment, side1) ||
        CheckCollisionLineSegment(segment, side2) ||
        CheckCollisionLineSegment(segment, side3) ||
        CheckCollisionLineSegment(segment, side4))
    {
        return true;
    }

    return false;
}

bool CheckCollisionRecSegment(LineSegment segment, Rectangle rec)
{
    // Check if any of the four corners of the rectangle are inside the line segment
    if (CheckCollisionPointSegment(rec.x, rec.y, segment) ||
        CheckCollisionPointSegment(rec.x + rec.width, rec.y, segment) ||
        CheckCollisionPointSegment(rec.x, rec.y + rec.height, segment) ||
        CheckCollisionPointSegment(rec.x + rec.width, rec.y + rec.height, segment))
    {
        return true;
    }

    // Check if the line segment intersects any of the four sides of the rectangle
    if (CheckCollisionLineRec(segment, rec))
    {
        return true;
    }

    return false;
}