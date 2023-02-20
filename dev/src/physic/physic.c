#include "../../../lib/raylib/src/raylib.h"

#include "physic.h"
#include "../line_segment/line_segment.h"

bool CollisionPhysic(Physic *phyA, Rectangle recA, Rectangle recB)
{
    bool collision = false;

    // Check for collisions between the two objects using a line segment
    Vector2 start = {recA.x, recA.y};
    Vector2 end = {recA.x + phyA->vel.x, recA.y + phyA->vel.y};
    LineSegment segment = {start, end};

    if (CheckCollisionRecs(recB, recA) || CheckCollisionRecSegment(segment, recB))
    {
        phyA->collision[0] = true;
        collision = true;

        Rectangle intersect = GetCollisionRec(recA, recB);

        // Reposition the objects so they no longer intersect
        if (intersect.width > intersect.height)
        {
            // Collision occurred on the top or bottom
            if (recA.y < recB.y)
            { // Top
                phyA->collision[1] = true;
                phyA->pos.y -= intersect.height;
            }
            else
            { // Bottom
                phyA->collision[2] = true;
                phyA->pos.y += intersect.height;
            }
        }
        else
        {
            // Collision occurred on the left or right side
            if (recA.x < recB.x)
            { // Left
                phyA->collision[3] = true;
                phyA->pos.x -= intersect.width;
            }
            else
            { // Right
                phyA->collision[4] = true;
                phyA->pos.x += intersect.width;
            }
        }

        phyA->vel.x *= 0.8f;
        phyA->vel.y *= 0.8f;
    }
    return collision;
}
