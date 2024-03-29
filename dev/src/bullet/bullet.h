#ifndef BULLET_H
#define BULLET_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "../../../lib/raylib/src/raylib.h"
#include "../physic/physic.h"

    typedef struct Bullet
    {
        int playerId;
        Physic p;
        Vector2 speed;
        float radian;
        bool direction;
        bool inactive;
        int timerInactive;
        bool isNew;
        Vector2 startPos;
        float distanceLaser; // I couldn't find anything better :/
        Color COLOR;
    } Bullet;

    void InitBullet(void);
    void UpdateBullet(Bullet *bullet);
    void BulletBounce(Bullet *bullet);
    void DrawBullet(Bullet bullet);
    void BulletValueToData(Bullet bullet, char *DataToSend);

#ifdef __cplusplus
}
#endif

#endif // BULLET_H
