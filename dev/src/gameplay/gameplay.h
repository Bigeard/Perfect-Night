#ifndef GAMEPLAY_H
#define GAMEPLAY_H

#include <stdbool.h>
#ifdef __cplusplus
extern "C"
{
#endif

#include "../../../lib/raylib/src/raylib.h"
#include "../player/player.h"

#define BLACKGROUND     \
    CLITERAL(Color)     \
    {                   \
        33, 37, 48, 255 \
    }
#define CHANGECOLOR        \
    CLITERAL(Color)        \
    {                      \
        255, 100, 255, 255 \
    }

    extern bool activeDev;
    extern float arenaSizeX;
    extern float arenaSizeY;
    extern Color themeColor[8];
    extern int ColorScore[8];

    extern Texture2D qrCodeTexture;

    extern Texture2D BonusAmmunitionTexture;
    extern Texture2D BonusLifeTexture;
    extern Texture2D BonusLifeWhiteTexture;
    extern Texture2D BonusSpeedTexture;
    extern Texture2D NothingTexture;

    extern Texture2D PattenSlashTexture;
    extern Texture2D PattenSquareTexture;
    extern Texture2D PattenCrossTexture;

    extern Player players[8];
    extern int numberPlayer;

    extern Player *outsidePlayer;
    extern Player *lastOutsidePlayer;

    void InitGameplay(void);
    void UpdateGameplay(void);
    void ResetGame(void);
    void DrawGameplay(void);
    void DrawGameArena(void);
    void DrawPauseGame(void);
    void GenerateQrCode(void);
    void QrCodeParticleInit(float *qrCodeParticlesAnimationTimer, int *qrCodeParticlesIdColor, Particle *qrCodeParticles);

#ifdef __cplusplus
}
#endif

#endif // GAMEPLAY_H
