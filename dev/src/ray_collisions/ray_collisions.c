#include "../../../lib/raylib/src/raylib.h"
#include "../../../lib/raylib/src/raymath.h"

/*
 * https://gist.github.com/Pikachuxxxx/0dda4b70bf71b794b08923df34961844
 * With optimization
 */

static inline void SwapFloatValue(float *a, float *b)
{
    float t = *a;
    *a = *b;
    *b = t;
}

static inline float maxF(float num1, float num2)
{
    return (num1 > num2) ? num1 : num2;
}

static inline float minF(float num1, float num2)
{
    return (num1 > num2) ? num2 : num1;
}

bool RayVsRect2D(const Vector2 ray_origin, const Vector2 ray_dir, const Rectangle targetRect, Vector2 *contact_point, Vector2 *contact_normal, float *near_contact_time, Vector2 probableContactPoints[])
{
    const float delta_t1_X = targetRect.x - ray_origin.x;
    float t_hit_near_X = delta_t1_X / ray_dir.x;
    const float delta_t1_Y = targetRect.y - ray_origin.y;
    float t_hit_near_Y = delta_t1_Y / ray_dir.y;
    const float delta_t2_X = targetRect.x + targetRect.width - ray_origin.x;
    float t_hit_far_X = delta_t2_X / ray_dir.x;
    const float delta_t2_Y = targetRect.y + targetRect.height - ray_origin.y;
    float t_hit_far_Y = delta_t2_Y / ray_dir.y;

    if (t_hit_near_X > t_hit_far_X) {
        SwapFloatValue(&t_hit_near_X, &t_hit_far_X);
    }
    if (t_hit_near_Y > t_hit_far_Y) {
        SwapFloatValue(&t_hit_near_Y, &t_hit_far_Y);
    }

    probableContactPoints[0] = (Vector2){0, 0};
    probableContactPoints[1] = (Vector2){0, 0};

    if (!(t_hit_near_X < t_hit_far_Y && t_hit_near_Y < t_hit_far_X)) {
        return false;
    }

    *near_contact_time = maxF(t_hit_near_X, t_hit_near_Y);
    const float t_hit_far = minF(t_hit_far_X, t_hit_far_Y);

    const float Hit_Near_X_Position = ray_origin.x + (ray_dir.x * (*near_contact_time));
    const float Hit_Near_Y_Position = ray_origin.y + (ray_dir.y * (*near_contact_time));

    const float Hit_Far_X_Position = ray_origin.x + (ray_dir.x * t_hit_far);
    const float Hit_Far_Y_Position = ray_origin.y + (ray_dir.y * t_hit_far);

    const Vector2 Near_Hit_Vector = (Vector2){Hit_Near_X_Position, Hit_Near_Y_Position};
    const Vector2 Far_Hit_Vector = (Vector2){Hit_Far_X_Position, Hit_Far_Y_Position};

    probableContactPoints[0] = Near_Hit_Vector;
    probableContactPoints[1] = Far_Hit_Vector;

    if (t_hit_far < 0) {
        return false;
    }

    *contact_point = (Vector2){Hit_Near_X_Position, Hit_Near_Y_Position};

    if (t_hit_near_X > t_hit_near_Y) {
        *contact_normal = (ray_dir.x < 0) ? (Vector2){1, 0} : (Vector2){-1, 0};
    } else if (t_hit_near_X < t_hit_near_Y) {
        *contact_normal = (ray_dir.y < 0) ? (Vector2){0, 1} : (Vector2){0, -1};
    }

    return true;
}

bool DynamicRectVsRect(const Rectangle sourceRect, const Vector2 sourceRectVelocity, const Rectangle targetRect, Vector2 *contact_point, Vector2 *contact_normal, float *near_contact_time, Vector2 probableContactPoints[])
{
    const bool isMoving = (sourceRectVelocity.x != 0.0f || sourceRectVelocity.y != 0.0f);
    if (!isMoving)
    {
        return false;
    }

    const Rectangle expanded_target = {
        .x = targetRect.x - (sourceRect.width / 2),
        .y = targetRect.y - (sourceRect.height / 2),
        .width = targetRect.width + sourceRect.width,
        .height = targetRect.height + sourceRect.height
    };

    const Vector2 ray_or = {sourceRect.x + (sourceRect.width / 2), sourceRect.y + (sourceRect.height / 2)};
    const Vector2 ray_di = sourceRectVelocity;
    if (RayVsRect2D(ray_or, ray_di, expanded_target, contact_point, contact_normal, near_contact_time, probableContactPoints))
    {
        return true;
    }

    return false;
}
