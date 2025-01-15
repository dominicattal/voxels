#ifndef VEC_H
#define VEC_H

#include "type.h"

typedef struct {
    f32 x, y, z;
} vec3;

typedef struct {
    vec3 normal;
    f32 distance;
} Plane;

typedef struct {
    vec3 origin;
    vec3 direction;
} Ray;

typedef struct {
    vec3 a, b, c;
} Triangle;

typedef struct {
    vec3 a, b;
} Segment;

vec3 vec3_create(f32 x, f32 y, f32 z);
vec3 vec3_add(vec3 vec1, vec3 vec2);
vec3 vec3_sub(vec3 vec1, vec3 vec2);
vec3 vec3_normalize(vec3 vec);
vec3 vec3_scale(vec3 vec, f32 scale);
vec3 vec3_cross(vec3 vec1, vec3 vec2);
f32  vec3_dot(vec3 vec1, vec3 vec2);
f32  vec3_mag(vec3 vec);
void vec3_print(vec3 vec);

Ray ray_create(vec3 origin, vec3 direction);

Segment segment_create(vec3 a, vec3 b);

Triangle triangle_create(vec3 a, vec3 b, vec3 c);

f32  ray_intersects_triangle(Ray ray, Triangle triangle);
bool segment_intersects_triangle(Segment segment, Triangle triangle);
bool triangle_intersects_triangle(Triangle triangle1, Triangle triangle2);

#endif
