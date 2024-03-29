#ifndef PARTICLE_H
#define PARTICLE_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "../../../lib/raylib/src/raylib.h"
#include "../physic/physic.h"

    typedef struct Particle
    {
        Physic p;
        Color color;
        float initTimer;
        float timer;
    } Particle;

    void InitParticles(Vector2 origin, Vector2 originVelocity, float speedVelocity, Color color, float timer, Particle *particles, int sizeArray);
    void UpdateParticles(Particle *particles, int sizeArray);
    void DrawParticles(Particle *particles, int sizeArray);

#ifdef __cplusplus
}
#endif

#endif // PARTICLE_H
