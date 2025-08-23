#ifndef SCENE_3D_H
#define SCENE_3D_H
#include <stdint>
#include <math.h> // for sqrt

typedef struct
{
    int32_t x;
    int32_t y;
    int32_t z;
} Point3d;

typedef struct
{
    int32_t vx;
    int32_t vy;
    int32_t vz;
} Motion3d;

typedef struct
{
    Point3d *pos;
    Motion3d *vel;
    int8_t rad; // for circle like objects.
} Object3d;

inline bool doesPointIntersect(Object3d a, Object3d b)
{
    if (!a.pos || !b.pos)
        return false; // safety check

    int32_t dx = a.pos->x - b.pos->x;
    int32_t dy = a.pos->y - b.pos->y;
    int32_t dz = a.pos->z - b.pos->z;

    float dist = sqrtf(dx * dx + dy * dy + dz * dz);
    return dist <= (a.rad + b.rad);
}
// slighly inncaurte , overguesses well diagnal distances. , meant for MCU's with no FPU.
inline bool QuickdoesPointIntersect(Object3d a, Object3d b)
{
    if (!a.pos || !b.pos)
        return false; // safety check

    int32_t dx = a.pos->x - b.pos->x;
    int32_t dy = a.pos->y - b.pos->y;
    int32_t dz = a.pos->z - b.pos->z;

    int32_t dist = abs(dx) + abs(dy) + abs(dz);
    return dist <= (a.rad + b.rad);
}
#endif
