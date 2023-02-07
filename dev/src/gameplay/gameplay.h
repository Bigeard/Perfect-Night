#ifndef GAMEPLAY_H
#define GAMEPLAY_H

#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif

#include "../../../lib/raylib/src/raylib.h"
#include "../player/player.h"

extern bool activeDev;
extern float arenaSizeX;
extern float arenaSizeY;
extern Color themeColor[8];

extern Texture2D qrCodeTexture;
extern Texture2D BonusLifeTexture;
extern Texture2D BonusAmmunitionTexture;
extern Texture2D BonusSpeedTexture;

extern Player players[8];
extern Player *outsidePlayer;
extern Player *lastOutsidePlayer;

void InitGameplay(void);
void UpdateGameplay(void);
void DrawGameplay(void);
void DrawGameArena(void);
void DrawPauseGame(void);
void GenerateQrCode(void);

#ifdef __cplusplus
}
#endif

#endif // GAMEPLAY_H
