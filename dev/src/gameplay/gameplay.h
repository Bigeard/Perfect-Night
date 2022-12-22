#ifndef GAMEPLAY_H
#define GAMEPLAY_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../../../lib/raylib/src/raylib.h"


extern bool activeDev;
extern float arenaSizeX;
extern float arenaSizeY;
extern Texture2D qrCodeTexture;

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
