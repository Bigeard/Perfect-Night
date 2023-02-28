#include "../../../../../lib/raylib/src/raylib.h"

#include "./nothing.h"
#include "../../item.h"
#include "../../../gameplay/gameplay.h"
#include "../../../player/player.h"
#include "../../../tool/tool.h"

Item InitItemNothing(int player_id)
{
    return (Item){
        player_id,
        "Nothing",
        true,
        GetTime(),
        6.0,
        false,
        false,
        ShootItemNothing,
        UpdateItemNothing,
        DrawItemNothing,
        {0}};
}

void ShootItemNothing(Item *item, float calcPosRadianX, float calcPosRadianY, float delta_x, float delta_y)
{
    if (!item->active)
        return;
    Player *player = &players[item->player_id - 1];
    player->ammunition++;
}

void UpdateItemNothing(Item *item)
{
    if (!item->active)
        return;
    double elapsedTime = GetTime() - item->timer;
    if (item->maxTimer < elapsedTime)
    {
        item->active = false;
    }
}

void DrawItemNothing(Item *item)
{
    if (!item->active)
        return;
    Player player = players[item->player_id - 1];

    // Init Color
    Color color = player.color;
    Color whiteColor = WHITE;

    if (player.invincible)
    {
        // Lighten color
        color = LightenColor(color, 0.5);
        whiteColor = Fade(WHITE, 0.5);
    }

    // Draw Border of the Tank
    DrawTextureEx(playerBorderTexture, (Vector2){player.p.pos.x, player.p.pos.y}, 0, 1, whiteColor);

    // Draw Body / Template of the tank
    DrawTextureEx(playerBodyTexture, (Vector2){player.p.pos.x + 3, player.p.pos.y + 3}, 0, 1, color);
    DrawTexturePro(playerTemplatesTextures[player.id], (Rectangle){0, 0, 32, 32}, (Rectangle){player.p.pos.x + 20, player.p.pos.y + 20, 32, 32}, (Vector2){16, 16}, player.radian * (180 / PI) + 90, WHITE);

    // Animation pick-up item
    double elapsedTime = GetTime() - item->timer;
    if (elapsedTime <= 4.0)
    {
        DrawTexture(
            NothingTexture,
            (player.p.pos.x + player.p.size.x / 2) - 15,
            player.p.pos.y - 37,
            Fade(player.color, 1 - (elapsedTime / 4)));
    }
}
