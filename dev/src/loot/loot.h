#ifndef LOOT_H
#define LOOT_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "../../../lib/raylib/src/raylib.h"
#include "../player/player.h"
#include "../physic/physic.h"

    typedef struct Loot
    {
        int id;
        Physic p;
        bool active;
        int type;

        // Reappearance of the loot
        double timer;
        float delay;

        // Animation
        float animationTimer;
        int idColorA;
        int idColorB;
    } Loot;

    void InitLoot(void);
    void UpdateLoot(Loot *loot, Player *player);
    void DrawLoot(Loot loot);

#ifdef __cplusplus
}
#endif

#endif // LOOT_H
