#ifndef VEC_H
#define VEC_H

#include "type.h"

typedef struct {
    f32 x, y, z;
} vec3;

vec3 vec3_create(f32 x, f32 y, f32 z);
vec3 vec3_add(vec3 vec1, vec3 vec2);
vec3 vec3_sub(vec3 vec1, vec3 vec2);
vec3 vec3_normalize(vec3 vec);
vec3 vec3_scale(vec3 vec, f32 scale);
vec3 vec3_cross(vec3 vec1, vec3 vec2);
f32  vec3_dot(vec3 vec1, vec3 vec2);
f32  vec3_mag(vec3 vec);
void vec3_print(vec3 vec);

#endif
