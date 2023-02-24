#ifndef RAY_COLLISION_H
#define RAY_COLLISION_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "../../../lib/raylib/src/raylib.h"

    void SwapFloatValue(float *a, float *b);
    void SwapIntValues(int *a, int *b);
    int maxI(int num1, int num2);
    int minI(int num1, int num2);
    float maxF(float num1, float num2);
    float minF(float num1, float num2);
    int absI(int num);
    float absF(float num);

    bool RayVsRect2D(
        const Vector2 ray_origin,
        const Vector2 ray_dir,
        const Rectangle targetRect,
        Vector2 *contact_point,
        Vector2 *contact_normal,
        float *near_contact_time,
        Vector2 probableContactPoints[]);
    
    bool DynamicRectVsRect(
        const Rectangle sourceRect,
        const Vector2 sourceRectVelocity,
        const Rectangle targetRect, Vector2 *contact_point, Vector2 *contact_normal,
        float *near_contact_time, Vector2 probableContactPoints[]);

#ifdef __cplusplus
}
#endif

#endif // RAY_COLLISION_H
