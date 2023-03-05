#ifndef GAMEPLAY_H
#define GAMEPLAY_H

#include <stdbool.h>
#ifdef __cplusplus
extern "C"
{
#endif

#include "../../../lib/raylib/src/raylib.h"
#include "../player/player.h"

#define NUMBER_EIGHT 8
#define BLACKGROUND     \
    CLITERAL(Color)     \
    {                   \
        33, 37, 48, 255 \
    }
#define GRAYDARK     \
    CLITERAL(Color)     \
    {                   \
        86, 86, 86, 255 \
    }

    extern bool activeDev;
    extern bool activePerf;
    extern float arenaSizeX;
    extern float arenaSizeY;
    extern Color themeColor[NUMBER_EIGHT];
    extern int colorScore[NUMBER_EIGHT];

    extern int BoxesScoreFontSize[NUMBER_EIGHT];
    extern Vector2 BoxesScoreSize[NUMBER_EIGHT];

    extern Texture2D qrCodeTexture;

    extern Texture2D BonusAmmunitionTexture;
    extern Texture2D BonusLifeTexture;
    extern Texture2D BonusLifeWhiteTexture;
    extern Texture2D BonusSpeedTexture;
    extern Texture2D NothingTexture;

    extern Texture2D PattenSlashTexture;
    // extern Texture2D PattenSquareTexture;
    // extern Texture2D PattenCrossTexture;

    // extern Shader neonShader;

    extern Player players[NUMBER_EIGHT];
    extern int numberPlayer;

    extern Player *outsidePlayer;
    extern Player *lastOutsidePlayer;

    void InitGameplay(void);
    void InitMap(void);
    void UpdateGameplay(void);
    void ResetGame(void);
    void DrawGameplay(void);
    void DrawGameArena(void);
    void GenerateQrCode(void);
    void QrCodeParticleInit(float *qrCodeParticlesAnimationTimer, int *qrCodeParticlesIdColor, Particle *qrCodeParticles);

#ifdef __cplusplus
}
#endif

#endif // GAMEPLAY_H
