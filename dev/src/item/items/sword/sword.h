#ifndef SWORD_H
#define SWORD_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "../../../../../lib/raylib/src/raylib.h"

    typedef struct Sword
    {
        double lastSwingTime;
        float swingRadian;
    } Sword;

#ifdef __cplusplus
}
#endif

#endif // SWORD_H
