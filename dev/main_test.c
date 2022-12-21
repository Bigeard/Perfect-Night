#include "../lib/raylib/src/raylib.h"
#include "src/gameplay/gameplay.h"
#include <emscripten/emscripten.h>
#include <math.h>



static void GameMainLoop(void);
float Vector2CrossProduct(Vector2 a, Vector2 b);
Vector2 Vector2SubtractVector2(Vector2 a, Vector2 b);
bool CheckCollisionLineSegment(LineSegment a, LineSegment b);
bool CheckCollisionPointSegment(float pointX, float pointY, LineSegment segment);
bool CheckCollisionLineRec(LineSegment segment, Rectangle rec);
bool CheckCollisionRecSegment(LineSegment segment, Rectangle rec);

Rectangle obj1 = { 200, 300, 50, 50 };

Rectangle objCol[31] = {
    { 1000, 300, 50, 50 },
    { 500, 350, 50, 50 },
    { 550, 350, 50, 50 },
    { 700, 349.5, 50, 50 },
    { 10, 300, 50, 50 },
    { 10, 350, 50, 50 },
    { 1200, 300, 50, 50 },
    { 10, 400, 50, 50 },
    { 10, 500, 50, 50 },
    { 10, 600, 50, 50 },
    { 10, 600, 50, 50 },
    { 10, 700, 50, 50 },
    { 10, 800, 50, 50 },
    { 10, 900, 50, 50 },
    { 10, 1000, 50, 50 },
    { 10, 1100, 50, 50 },
    { 10, 1200, 50, 50 },
    { 10, 1300, 50, 50 },
    { 10, 1400, 50, 50 },
    { 10, 1500, 50, 50 },
    { 10, 1600, 50, 50 },
    { 10, 1700, 50, 50 },
    { 10, 1800, 50, 50 },
    { 10, 1900, 50, 50 },
    { 10, 2000, 50, 50 },
    { 10, 2100, 50, 50 },
    { 10, 2200, 50, 50 },
    { 10, 2300, 50, 50 },
    { 10, 2400, 50, 50 },
    { 10, 2500, 50, 50 },
    { 10, 2600, 50, 50 },
    { 10, 2700, 50, 50 },
    { 10, 2800, 50, 50 },
    { 10, 2900, 50, 50 },
    { 10, 3000, 50, 50 }
};

Vector2 velocity = { 10, 0 }; // Movement speed of obj1

int main(void)
{
    InitWindow(1920, 1060, "Perfect Night");
    InitAudioDevice();
    InitGameplay();
    emscripten_set_main_loop(GameMainLoop, 120, 1);
    return 0;
}


// Function to compute the cross product of two vectors
float Vector2CrossProduct(Vector2 a, Vector2 b)
{
    return a.x * b.y - a.y * b.x;
}

// Function to subtract two vectors
Vector2 Vector2SubtractVector2(Vector2 a, Vector2 b)
{
    Vector2 result = { a.x - b.x, a.y - b.y };
    return result;
}

// Function to check if two line segments intersect
bool CheckCollisionLineSegment(LineSegment a, LineSegment b)
{
    Vector2 p = a.start;
    Vector2 r = Vector2SubtractVector2(a.end, a.start);
    Vector2 q = b.start;
    Vector2 s = Vector2SubtractVector2(b.end, b.start);

    float t = Vector2CrossProduct(Vector2SubtractVector2(q, p), s) / Vector2CrossProduct(r, s);
    float u = Vector2CrossProduct(Vector2SubtractVector2(q, p), r) / Vector2CrossProduct(r, s);

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
    LineSegment side1 = { { rec.x, rec.y }, { rec.x + rec.width, rec.y } };
    LineSegment side2 = { { rec.x + rec.width, rec.y }, { rec.x + rec.width, rec.y + rec.height } };
    LineSegment side3 = { { rec.x + rec.width, rec.y + rec.height }, { rec.x, rec.y + rec.height } };
    LineSegment side4 = { { rec.x, rec.y + rec.height }, { rec.x, rec.y } };

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

static void GameMainLoop(void)
{
    
        if (IsKeyDown(KEY_UP)) {
            velocity.x += velocity.x > 0 ? 0.1 : -0.1;
        }
        if (IsKeyDown(KEY_DOWN)) {
            velocity.x += velocity.x > 0 ? -0.1 : 0.1;
        }
        obj1.x += velocity.x;
        obj1.y += velocity.y;

        // Check for collisions between the two objects using a line segment
        Vector2 start = { obj1.x, obj1.y };
        Vector2 end = { obj1.x + velocity.x, obj1.y + velocity.y };
        LineSegment segment = { start, end };

        for (int i = 0; i < 31; i++) {
            if (CheckCollisionRecs(objCol[i], obj1) || CheckCollisionRecSegment(segment, objCol[i]))
            {
                // Collision detected! Adjust the position of obj1 to stop it from passing through objCol[j]
                if (velocity.x > 0) obj1.x = objCol[i].x - obj1.width;
                else if (velocity.x < 0) obj1.x = objCol[i].x + objCol[i].width;
                if (velocity.y > 0) obj1.y = objCol[i].y - obj1.height;
                else if (velocity.y < 0) obj1.y = objCol[i].y + objCol[i].height;

                // Reverse the velocity to bounce the object off of objCol[j]
                velocity.x = -velocity.x;
                velocity.y = -velocity.y;
            }
        }

        BeginDrawing();

        ClearBackground(RAYWHITE);

        // Draw the two objects
        for (int j = 0; j < 31; j++) {
            DrawRectangleRec(objCol[j], BLUE);
        }
        DrawRectangleRec(obj1, RED);

        DrawText(TextFormat("Pos X: %f", obj1.x), 120, 120, 20, GREEN);
        DrawText(TextFormat("Pos Y: %f", obj1.x), 120, 200, 20, GREEN);
        DrawText(TextFormat("Velocity X: %f", velocity.x), 120, 20, 20, GREEN);

        EndDrawing();

        DrawFPS(20, 20);
    // UpdateGameplay();
    // DrawGameplay();
}
