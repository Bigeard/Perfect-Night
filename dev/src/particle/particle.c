#include "../../../lib/raylib/src/raylib.h"
#include "../../../lib/raylib/src/raymath.h"
#include "particle.h"
#include <string.h>


void InitParticles(Vector2 origin, Vector2 velocity, Color color, Particle *particles, int sizeArray) {
    for (int i = 0; i < sizeArray; i++) {
        float size = (float)GetRandomValue(40, 60) / 10.0f;
        particles[i].p.pos = origin;
        particles[i].p.vel = (Vector2){ (float)GetRandomValue(velocity.x - 8, velocity.x  + 8)/10.0f, (float)GetRandomValue(velocity.y - 8, velocity.y + 8)/10.0f };
        particles[i].color = color;
        particles[i].p.size = (Vector2){ size, size };
        particles[i].timer = 120.0f;
    }
}

void UpdateParticles(Particle *particles, int sizeArray) {
    for (int i = 0; i < sizeArray; i++) {
        particles[i].p.pos = Vector2Add(particles[i].p.pos, particles[i].p.vel);
        particles[i].timer--;
        if (particles[i].timer <= 0.0f) {
            // @TODO Check if it is worth importing string.h
            memset(particles, 0, sizeof(&particles));
        }
    }
}

void DrawParticles(Particle *particles, int sizeArray) {
    for (int i = 0; i < sizeArray; i++) {
        DrawCircle(
            particles[i].p.pos.x, 
            particles[i].p.pos.y, 
            particles[i].p.size.x,
            Fade(particles[i].color, particles[i].timer/120.0f)
        );
    }
}
