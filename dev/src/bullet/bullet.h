#ifndef BULLET_H
#define BULLET_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../physic/physic.h"


typedef struct Bullet {
    int playerId;
    Physic p;
    Vector2 speed;
    float radian;
    bool direction;
    Color COLOR;
} Bullet;

void InitBullet(void);
void UpdateBullet(Bullet *bullet);
void BulletBounce(Bullet *bullet);
void DrawBullet(Bullet bullet);


#ifdef __cplusplus
}
#endif

#endif // BULLET_H
