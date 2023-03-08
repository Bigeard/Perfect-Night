#include "../../../../../lib/raylib/src/raylib.h"

#include "./bonus_life.h"
#include "../../item.h"
#include "../../../player/player.h"
#include "../../../gameplay/gameplay.h"
#include "../../../tool/tool.h"

Item InitItemBonusLife(int player_id)
{
    Item item = {
        player_id,
        "Bonus Life",
        true,
        GetTime(),
        4.0,
        true,
        true,
        ShootItemBonusLife,
        UpdateItemBonusLife,
        DrawItemBonusLife,
        {1}};
    Player *player = &players[item.player_id - 1];
    player->life++;
    GamepadPlayerLife(player->gamepadId, player->life);
    return item;
}

void ShootItemBonusLife(Item *item, float calcPosRadianX, float calcPosRadianY, float delta_x, float delta_y)
{
    // Nothing
}

void UpdateItemBonusLife(Item *item)
{
    if (!item->active)
        return;
    if (item->maxTimer < GetTime() - item->timer)
    {
        item->active = false;
    }
}

void DrawItemBonusLife(Item *item)
{
    if (!item->active)
        return;
    const Player player = players[item->player_id - 1];
    DrawTexture(
        BonusLifeWhiteTexture,
        (player.p.pos.x + player.p.size.x / 2) - 16,
        player.p.pos.y - 35,
        Fade(WHITE, 0.9 - ((GetTime() - item->timer) / item->maxTimer)));
    DrawTexture(
        BonusLifeTexture,
        (player.p.pos.x + player.p.size.x / 2) - 16,
        player.p.pos.y - 35,
        Fade(player.color, 0.9 - ((GetTime() - item->timer) / item->maxTimer)));
}
