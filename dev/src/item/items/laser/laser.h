#ifndef LASER_H
#define LASER_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "../../../../../lib/raylib/src/raylib.h"

    typedef struct Laser
    {
        float chargeTimerLaser;
        float laserStartPosX;
        float laserStartPosY;
        float laserEndPosX;
        float laserEndPosY;
        float laserRadian;
        float animationTimerLaser;
    } Laser;

#ifdef __cplusplus
}
#endif

#endif // LASER_H
