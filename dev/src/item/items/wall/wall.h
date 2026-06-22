#ifndef WALL_H
#define WALL_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "../../../../../lib/raylib/src/raylib.h"

    typedef struct Wall
    {
        bool active;
        Vector2 start;
        Vector2 end;
    } Wall;

    struct Player;
    struct Bullet;
    void DrawPlayerWall(const struct Player *player);
    bool ReflectBulletFromWall(struct Bullet *bullet, const struct Player *owner);
    bool ResolvePlayerWallCollision(struct Player *player, const struct Player *owner);

#ifdef __cplusplus
}
#endif

#endif // WALL_H
