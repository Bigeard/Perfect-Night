#ifndef PHYSIC_H
#define PHYSIC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../../../lib/raylib/src/raylib.h"

typedef struct Physic {
    Vector2 pos;
    Vector2 size;
    Vector2 vel;
    bool collision[5];
} Physic;

void CollisionPhysic(Physic *phyA, Rectangle recA, Rectangle recB);


#ifdef __cplusplus
}
#endif

#endif // PHYSIC_H
