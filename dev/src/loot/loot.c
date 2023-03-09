#include "../../../lib/raylib/src/raylib.h"

#include "loot.h"
#include "../gameplay/gameplay.h"
#include "../player/player.h"
#include "../tool/tool.h"

Texture2D LootBoxTexture;

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
        enum TypeItem type = loot->type;
        InitItemWithTypeItem(player->id, type, -1);
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
