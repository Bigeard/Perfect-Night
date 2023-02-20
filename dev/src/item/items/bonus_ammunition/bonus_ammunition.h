#ifndef BONUS_AMMUNITION_H
#define BONUS_AMMUNITION_H

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct BonusAmmunition
    {
        int delayReload;
        // Animation
        float animationTimer;
        int idColorA;
        int idColorB;
    } BonusAmmunition;

#ifdef __cplusplus
}
#endif

#endif // BONUS_AMMUNITION_H
