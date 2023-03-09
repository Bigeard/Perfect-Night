#include "../../../../../lib/raylib/src/raylib.h"

#include "./bonus_speed.h"
#include "../../item.h"
#include "../../../gameplay/gameplay.h"
#include "../../../player/player.h"

Item InitItemBonusSpeed(int player_id, float maxTimer)
{
    float defaultMaxTimer = 6.0f;
    if (maxTimer > -1.0f)
        defaultMaxTimer = maxTimer;

    Item item = {
        player_id,
        "Bonus Speed",
        true,
        GetTime(),
        defaultMaxTimer,
        true,
        true,
        ShootItemBonusSpeed,
        UpdateItemBonusSpeed,
        DrawItemBonusSpeed,
        {0, 0.9f}};
    Player *player = &players[player_id - 1];
    player->speed.x = player->speed.x + item.bonusSpeed.speed;
    player->speed.y = player->speed.y + item.bonusSpeed.speed;
    return item;
}

void ShootItemBonusSpeed(Item *item, float calcPosRadianX, float calcPosRadianY, float delta_x, float delta_y)
{
    // Nothing
}

void UpdateItemBonusSpeed(Item *item)
{
    if (!item->active)
    {
        Player *player = &players[item->player_id - 1];
        if (player->speed.x > 3.05f || player->speed.y > 3.05f)
        {
            player->speed.x = 3.05f;
            player->speed.y = 3.05f;
        }
        return;
    };
    if (item->maxTimer < GetTime() - item->timer)
    {
        item->active = false;
    }
    Player *player = &players[item->player_id - 1];
    if (player->speed.x <= 3.05f && player->speed.y <= 3.05f)
    {
        player->speed.x = player->speed.x + item->bonusSpeed.speed;
        player->speed.y = player->speed.y + item->bonusSpeed.speed;
    }
}

void DrawItemBonusSpeed(Item *item)
{
    if (!item->active)
        return;
    const Player player = players[item->player_id - 1];
    const double elapsedTime = GetTime() - item->timer;
    if (elapsedTime <= 4.0)
    {
        DrawTexture(
            BonusSpeedTexture,
            (player.p.pos.x + player.p.size.x / 2) - 10,
            player.p.pos.y - 42,
            Fade(player.color, 1 - (elapsedTime / 4)));
    }
}
