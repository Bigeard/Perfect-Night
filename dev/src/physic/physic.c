#include "../../../lib/raylib/src/raylib.h"

#include "physic.h"
#include "../ray_collisions/ray_collisions.h"

void CollisionPhysic(Physic *phyA, Rectangle recA, Rectangle recB)
{
    // Ray Collision
    Vector2 contact_point = (Vector2){0.0f, 0.0f};
    Vector2 contact_normal = (Vector2){0.0f, 0.0f};
    float near_contact_time = 0.0f;
    Vector2 probableContactPoints[2];

    bool detectCollision = DynamicRectVsRect(recA, phyA->vel, recB, &contact_point, &contact_normal, &near_contact_time, probableContactPoints);
    if (detectCollision && near_contact_time < 1.0f)
    {
        phyA->collision[0] = true;
        Rectangle intersect = GetCollisionRec(recA, recB);

        if (contact_normal.y == -1) // TOP
        {
            phyA->collision[1] = true;
            phyA->pos.y -= intersect.height;
        }
        else if (contact_normal.y == 1) // BOTTOM
        {
            phyA->collision[2] = true;
            phyA->pos.y += intersect.height;
        }
        if (contact_normal.x == -1) // LEFT
        {
            phyA->collision[3] = true;
            phyA->pos.x -= intersect.width;
        }
        else if (contact_normal.x == 1)
        { // Right
            phyA->collision[4] = true;
            phyA->pos.x += intersect.width;
        }

        phyA->vel.x *= 0.8f;
        phyA->vel.y *= 0.8f;
    }
}
