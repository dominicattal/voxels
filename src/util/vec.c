#include "vec.h"
#include <math.h>
#include <stdio.h>

vec3 vec3_create(f32 x, f32 y, f32 z)
{
    return (vec3) { x, y, z };
}

vec3 vec3_add(vec3 vec1, vec3 vec2)
{
    return (vec3) { vec1.x + vec2.x, vec1.y + vec2.y, vec1.z + vec2.z };
}

vec3 vec3_sub(vec3 vec1, vec3 vec2)
{
    return (vec3) { vec1.x - vec2.x, vec1.y - vec2.y, vec1.z - vec2.z };
}

vec3 vec3_normalize(vec3 vec)
{
    f32 mag = vec3_mag(vec);
    if (mag == 0)
        return vec;
    return (vec3) { vec.x / mag, vec.y / mag, vec.z / mag };
}

vec3 vec3_scale(vec3 vec, f32 scale)
{
    return (vec3) { vec.x * scale, vec.y * scale, vec.z * scale };
}

vec3 vec3_cross(vec3 vec1, vec3 vec2)
{
    return (vec3) {
        vec1.y * vec2.z - vec1.z * vec2.y,
        vec1.z * vec2.x - vec1.x * vec2.z,
        vec1.x * vec2.y - vec1.y * vec2.x
    };
}

f32 vec3_dot(vec3 vec1, vec3 vec2)
{
    return vec1.x * vec2.x + vec1.y * vec2.y + vec1.z * vec2.z;
}

f32  vec3_mag(vec3 vec)
{
    return sqrt(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
}

void vec3_print(vec3 vec)
{
    printf("(%.5f,%.5f,%.5f)\n", vec.x, vec.y, vec.z);
}