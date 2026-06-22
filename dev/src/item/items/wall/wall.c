#include "../../../../../lib/raylib/src/raylib.h"
#include "../../../../../lib/raylib/src/raymath.h"

#include "./wall.h"
#include "../../item.h"
#include "../../../gameplay/gameplay.h"
#include "../../../player/player.h"

#define WALL_DISTANCE 72.0f
#define WALL_HALF_LENGTH 52.0f
#define WALL_THICKNESS 12.0f

Item InitItemWall(int player_id, float maxTimer)
{
    float defaultMaxTimer = 10.0f;
    if (maxTimer > -1.0f)
        defaultMaxTimer = maxTimer;

    return (Item){
        .player_id = player_id,
        .type = WALL,
        .active = true,
        .timer = GetTime(),
        .maxTimer = defaultMaxTimer,
        .defaultShoot = true,
        .defaultDisplay = true,
        .ShootItem = ShootItemWall,
        .UpdateItem = UpdateItemWall,
        .DrawItem = DrawItemWall};
}

void ShootItemWall(Item *item, float calcPosRadianX, float calcPosRadianY, float delta_x, float delta_y)
{
    if (!item->active) return;

    Player *player = &players[item->player_id - 1];

    const bool placeWall = player->charge <= 4.0f;
    
    if(placeWall) {
        const Vector2 direction = {cosf(player->lastRadian), sinf(player->lastRadian)};
        const Vector2 normal = {-direction.y, direction.x};
        const Vector2 playerCenter = {
            player->p.pos.x + player->p.size.x/2.0f,
            player->p.pos.y + player->p.size.y/2.0f};
        const Vector2 wallCenter = Vector2Add(playerCenter, Vector2Scale(direction, WALL_DISTANCE));

        player->wall.active = true;
        player->wall.start = Vector2Add(wallCenter, Vector2Scale(normal, WALL_HALF_LENGTH));
        player->wall.end = Vector2Subtract(wallCenter, Vector2Scale(normal, WALL_HALF_LENGTH));

        // A normal pickup places one wall. Infinite-item mode can replace it repeatedly.
        if (item->maxTimer < FLT_MAX) item->active = false;

        item->defaultShoot = false;
    }
    else item->defaultShoot = true;
}

void UpdateItemWall(Item *item)
{
    if (!item->active)
        return;
    if (item->maxTimer < GetTime() - item->timer)
        item->active = false;
}

void DrawItemWall(Item *item)
{
    (void)item;
}

void DrawPlayerWall(const Player *player)
{
    if (!player->wall.active)
        return;
    DrawLineEx(player->wall.start, player->wall.end, WALL_THICKNESS + 3.0f, WHITE);
    DrawLineEx(player->wall.start, player->wall.end, WALL_THICKNESS - 3.0f, player->color);
}

bool ReflectBulletFromWall(Bullet *bullet, const Player *owner)
{
    if (!owner->wall.active || bullet->inactive || bullet->distanceLaser > 0.0f)
        return false;

    const Vector2 center = {
        bullet->p.pos.x + bullet->p.size.x,
        bullet->p.pos.y + bullet->p.size.y};
    const Vector2 previousCenter = Vector2Subtract(center, bullet->p.vel);
    Vector2 collisionPoint = {0.0f, 0.0f};
    const bool crossed = CheckCollisionLines(
        previousCenter, center, owner->wall.start, owner->wall.end, &collisionPoint);
    const bool touching = CheckCollisionCircleLine(
        center, bullet->p.size.x + WALL_THICKNESS/2.0f, owner->wall.start, owner->wall.end);
    if (!crossed && !touching)
        return false;

    Vector2 wallDirection = Vector2Subtract(owner->wall.end, owner->wall.start);
    const float wallLengthSquared = Vector2LengthSqr(wallDirection);
    if (wallLengthSquared <= 0.0f)
        return false;

    if (!crossed)
    {
        const float progress = Clamp(
            Vector2DotProduct(Vector2Subtract(center, owner->wall.start), wallDirection)/wallLengthSquared,
            0.0f,
            1.0f);
        collisionPoint = Vector2Add(owner->wall.start, Vector2Scale(wallDirection, progress));
    }
    AddBulletLineImpact(owner->wall.start, owner->wall.end, collisionPoint, bullet->COLOR);

    wallDirection = Vector2Normalize(wallDirection);
    Vector2 normal = {-wallDirection.y, wallDirection.x};
    if (Vector2DotProduct(bullet->p.vel, normal) > 0.0f)
        normal = Vector2Negate(normal);

    bullet->p.vel = Vector2Reflect(bullet->p.vel, normal);
    bullet->speed = bullet->p.vel;
    bullet->radian = atan2f(bullet->p.vel.y, bullet->p.vel.x);
    bullet->playerId = owner->id;
    bullet->COLOR = owner->color;
    bullet->isNew = false;

    const Vector2 safeCenter = Vector2Add(
        crossed ? collisionPoint : center,
        Vector2Scale(normal, bullet->p.size.x + WALL_THICKNESS/2.0f + 1.0f));
    bullet->p.pos = Vector2Subtract(safeCenter, bullet->p.size);
    return true;
}

bool ResolvePlayerWallCollision(Player *player, const Player *owner)
{
    if (!owner->wall.active || !player->id || player->life <= 0)
        return false;

    const Vector2 wallDirection = Vector2Subtract(owner->wall.end, owner->wall.start);
    const float wallLengthSquared = Vector2LengthSqr(wallDirection);
    if (wallLengthSquared <= 0.0f)
        return false;

    const Vector2 center = {
        player->p.pos.x + player->p.size.x/2.0f,
        player->p.pos.y + player->p.size.y/2.0f};
    const float progress = Clamp(
        Vector2DotProduct(Vector2Subtract(center, owner->wall.start), wallDirection)/wallLengthSquared,
        0.0f,
        1.0f);
    const Vector2 closest = Vector2Add(owner->wall.start, Vector2Scale(wallDirection, progress));
    Vector2 away = Vector2Subtract(center, closest);
    const float distance = Vector2Length(away);
    const float collisionDistance = fmaxf(player->p.size.x, player->p.size.y)/2.0f + WALL_THICKNESS/2.0f;
    if (distance >= collisionDistance)
        return false;

    if (distance > 0.001f)
    {
        away = Vector2Scale(away, 1.0f/distance);
    }
    else
    {
        const Vector2 normalizedWall = Vector2Normalize(wallDirection);
        away = (Vector2){-normalizedWall.y, normalizedWall.x};
        if (Vector2DotProduct(player->p.vel, away) > 0.0f)
            away = Vector2Negate(away);
    }

    const float penetration = collisionDistance - distance;
    player->p.pos = Vector2Add(player->p.pos, Vector2Scale(away, penetration + 0.5f));

    const float velocityIntoWall = Vector2DotProduct(player->p.vel, away);
    if (velocityIntoWall < 0.0f)
        player->p.vel = Vector2Subtract(player->p.vel, Vector2Scale(away, velocityIntoWall));
    return true;
}
