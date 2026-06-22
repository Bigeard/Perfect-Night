#ifndef BULLET_H
#define BULLET_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stddef.h>
#include "../../../lib/raylib/src/raylib.h"
#include "../physic/physic.h"

#define BULLET_TRAIL_LENGTH 10

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
        float explosionRadius;
        double explosionTime;
        Vector2 explosionPosition;
        Vector2 trail[BULLET_TRAIL_LENGTH];
        int trailCount;
        int trailIndex;
    } Bullet;

    void InitBullet(void);
    void UpdateBullet(Bullet *bullet);
    void BulletBounce(Bullet *bullet);
    void ExplodeBullet(Bullet *bullet);
    void DrawBullet(Bullet bullet);
    void AddBulletBoxImpact(const Bullet *bullet, Rectangle wall);
    void AddBulletLineImpact(Vector2 wallStart, Vector2 wallEnd, Vector2 contact, Color color);
    void DrawBulletImpacts(void);
    void BulletValueToData(Bullet bullet, char *DataToSend, size_t dataSize);

#ifdef __cplusplus
}
#endif

#endif // BULLET_H
