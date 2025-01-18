#ifndef CAMERA_H
#define CAMERA_H

#include "../util/util.h"

void camera_init(void);
void camera_move(vec3 mag, f32 dt);
void camera_rotate(f32 mag, f32 dt);
void camera_tilt(f32 mag, f32);
void camera_zoom(f32 mag, f32 dt);

vec3 camera_position(void);
vec3 camera_facing(void);
f32  camera_pitch(void);
f32  camera_yaw(void);
bool camera_aabb_in_frustrum(AABB aabb);

#endif