#ifndef PHYSIC_H
#define PHYSIC_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "../../../lib/raylib/src/raylib.h"

    typedef struct Physic
    {
        Vector2 pos;       // Position
        Vector2 size;      // Size
        Vector2 vel;       // Velocity
        bool collision[5]; // Collision: IsCollision, Up, Down, Left, Right
    } Physic;

    void CollisionPhysic(Physic *phyA, Rectangle recA, Rectangle recB);

#ifdef __cplusplus
}
#endif

#endif // PHYSIC_H
