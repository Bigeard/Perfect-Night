#include "../../../lib/raylib/src/raylib.h"
#include "../../../lib/raylib/src/raymath.h"

#include "loot.h"
#include "../gameplay/gameplay.h"
#include "../player/player.h"
#include "../tool/tool.h"

Texture2D LootBoxTexture;
enum Types
{
    RANDOM = 0,
    BONUS_AMMUNITION = 1,
    BONUS_LIFE = 2,
    BONUS_SPEED = 3,
    LASER = 4,
    MULTI_SHOOT = 5,
    ROCKET = 6,
    SWORD = 7,
    NOTHING = 8
};

void InitLoot(void)
{
    LootBoxTexture = LoadTexture("resources/loot_box.png");
}

void UpdateLoot(Loot *loot, Player *player)
{
    if (!loot->id)
        return;
    if (loot->timer != 0.0 && loot->delay <= (float)(GetTime() - loot->timer))
    {
        loot->active = true;
    }
    if (!loot->active)
        return;
    bool itemCollision = CheckCollisionRecs(
        (Rectangle){loot->p.pos.x, loot->p.pos.y, loot->p.size.x, loot->p.size.y},
        (Rectangle){player->p.pos.x, player->p.pos.y, player->p.size.x, player->p.size.y});
    if (itemCollision)
    {
        enum Types type = loot->type;
        if (RANDOM == loot->type)
        {
            type = GetRandomValue(1, 5); // Max = 5 and Min = 1
            if (activeDev)
            {
                type = 1; // @DEV
            }
        }
        switch (type)
        {
        case BONUS_AMMUNITION:
            player->item = InitItemBonusAmmunition(player->id);
            break;
        case BONUS_LIFE:
            player->item = InitItemBonusLife(player->id);
            break;
        case BONUS_SPEED:
            player->item = InitItemBonusSpeed(player->id);
            break;
        case LASER:
            // @TODO add feature laser
            // player->item = InitItemLaser(player->id);
            player->item = InitItemMultiShoot(player->id);
            break;
        case MULTI_SHOOT:
            player->item = InitItemMultiShoot(player->id);
            break;
        case ROCKET:
            player->item = InitItemRocket(player->id);
            break;
        case SWORD:
            player->item = InitItemSword(player->id);
            break;
        case NOTHING:
            player->item = InitItemNothing(player->id);
            break;
        case RANDOM:
            break;
        }
        loot->active = false;
        if (loot->delay >= 0)
        {
            loot->timer = GetTime();
        }
    }
    loot->animationTimer += 0.004f;
    if (loot->animationTimer >= 1.0f)
    {
        loot->idColorA = loot->idColorB;
        loot->idColorB++;
        if (loot->idColorB >= sizeof(themeColor) / sizeof(themeColor[0]))
        {
            loot->idColorB = 0;
        }
        loot->animationTimer = 0.0f;
    }
}

void DrawLoot(Loot loot)
{
    if (!loot.active)
        return;
    DrawPoly((Vector2){loot.p.pos.x + 32.0f, loot.p.pos.y + 32.0f}, 6, 25.0f, 0.0f, WHITE);
    DrawTexture(
        LootBoxTexture,
        loot.p.pos.x,
        loot.p.pos.y,
        LerpColor(
            themeColor[loot.idColorA],
            themeColor[loot.idColorB],
            loot.animationTimer));
}
