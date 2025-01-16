#include "camera.h"
#include "../renderer/renderer.h"
#include "../window/window.h"
#include <math.h>
#include <stdio.h>

#define NEAR_CLIP_DISTANCE  0.1f
#define FAR_CLIP_DISTANCE   1000.0f
#define DEFAULT_YAW         PI
#define DEFAULT_PITCH       0
#define DEFAULT_FOV         PI / 4
#define DEFAULT_ZOOM        15
#define DEFAULT_ROTSPEED    1
#define DEFAULT_MOVESPEED   100
#define Y_AXIS              vec3_create(0, 1, 0)

typedef struct {
    f32 yaw, pitch, zoom, fov, move_speed, rotate_speed;
    f32 view[16], proj[16];
    vec3 position, facing, right, up;
    Hexahedron frustrum;
} Camera;

static Camera camera;

static void view(f32 view[16], vec3 right, vec3 up, vec3 facing, vec3 position);
static void orthographic(f32 proj[16], f32 aspect_ratio, f32 zoom);
static void perspective(f32 proj[16], f32 aspect_ratio, f32 fov);

static void update_orientation_vectors(void)
{
    camera.facing.x = cos(camera.yaw) * cos(-camera.pitch);
    camera.facing.y = sin(-camera.pitch);
    camera.facing.z = sin(camera.yaw) * cos(-camera.pitch);
    camera.right = vec3_normalize(vec3_cross(Y_AXIS, camera.facing));
    camera.up = vec3_cross(camera.facing, camera.right);
}

static void update_frustrum(void)
{
    f32 horizontal_edge_length, vertical_edge_length;
    vec3 plane_center, edge_center;

    horizontal_edge_length = NEAR_CLIP_DISTANCE * tan(camera.fov / 2);
    vertical_edge_length = horizontal_edge_length / window_aspect_ratio();
    plane_center = vec3_add(camera.position, vec3_scale(camera.facing, NEAR_CLIP_DISTANCE));
    edge_center = vec3_sub(plane_center, vec3_scale(camera.right, horizontal_edge_length));
    camera.frustrum.points[0] = vec3_sub(edge_center, vec3_scale(camera.up, vertical_edge_length));
    camera.frustrum.points[1] = vec3_add(edge_center, vec3_scale(camera.up, vertical_edge_length));
    edge_center = vec3_add(plane_center, vec3_scale(camera.right, horizontal_edge_length));
    camera.frustrum.points[2] = vec3_add(edge_center, vec3_scale(camera.up, vertical_edge_length));
    camera.frustrum.points[3] = vec3_sub(edge_center, vec3_scale(camera.up, vertical_edge_length));
    
    horizontal_edge_length = FAR_CLIP_DISTANCE * tan(camera.fov / 2);
    vertical_edge_length = horizontal_edge_length / window_aspect_ratio();
    plane_center = vec3_add(camera.position, vec3_scale(camera.facing, FAR_CLIP_DISTANCE));
    edge_center = vec3_sub(plane_center, vec3_scale(camera.right, horizontal_edge_length));
    camera.frustrum.points[4] = vec3_sub(edge_center, vec3_scale(camera.up, vertical_edge_length));
    camera.frustrum.points[5] = vec3_add(edge_center, vec3_scale(camera.up, vertical_edge_length));
    edge_center = vec3_add(plane_center, vec3_scale(camera.right, horizontal_edge_length));
    camera.frustrum.points[6] = vec3_add(edge_center, vec3_scale(camera.up, vertical_edge_length));
    camera.frustrum.points[7] = vec3_sub(edge_center, vec3_scale(camera.up, vertical_edge_length));

    vec3 v1, v2;
    v1 = vec3_add(camera.position, vec3_scale(camera.facing, NEAR_CLIP_DISTANCE));
    camera.frustrum.planes[0].normal = camera.facing;
    camera.frustrum.planes[0].distance = -vec3_dot(v1, camera.frustrum.planes[0].normal);
    v1 = vec3_add(camera.position, vec3_scale(camera.facing, FAR_CLIP_DISTANCE));
    camera.frustrum.planes[1].normal = vec3_scale(camera.facing, -1);
    camera.frustrum.planes[1].distance = -vec3_dot(v1, camera.frustrum.planes[1].normal);
    v1 = vec3_sub(camera.frustrum.points[1], camera.position);
    v2 = vec3_sub(camera.frustrum.points[0], camera.position);
    camera.frustrum.planes[2].normal = vec3_normalize(vec3_cross(v1, v2));
    v1 = vec3_sub(camera.frustrum.points[3], camera.position);
    v2 = vec3_sub(camera.frustrum.points[2], camera.position);
    camera.frustrum.planes[3].normal = vec3_normalize(vec3_cross(v1, v2));
    v1 = vec3_sub(camera.frustrum.points[0], camera.position);
    v2 = vec3_sub(camera.frustrum.points[3], camera.position);
    camera.frustrum.planes[4].normal = vec3_normalize(vec3_cross(v1, v2));
    v1 = vec3_sub(camera.frustrum.points[2], camera.position);
    v2 = vec3_sub(camera.frustrum.points[1], camera.position);
    camera.frustrum.planes[5].normal = vec3_normalize(vec3_cross(v1, v2));
    for (i32 i = 2; i < 6; i++)
        camera.frustrum.planes[i].distance = -vec3_dot(camera.position, camera.frustrum.planes[i].normal);
}

static void update_view_matrix(void)
{
    view(camera.view, camera.right, camera.up, camera.facing, camera.position);
    renderer_uniform_update_view(camera.view);
}

static void update_proj_matrix()
{
    perspective(camera.proj, window_aspect_ratio(), camera.fov);
    renderer_uniform_update_proj(camera.proj);
}

void camera_init(void)
{
    camera.yaw = DEFAULT_YAW;
    camera.pitch = DEFAULT_PITCH;
    camera.zoom = DEFAULT_ZOOM;
    camera.fov = DEFAULT_FOV;
    camera.move_speed = DEFAULT_MOVESPEED;
    camera.rotate_speed = DEFAULT_ROTSPEED;
    camera.position = vec3_create(5, 5, 5);
    update_orientation_vectors();
    update_view_matrix();
    update_proj_matrix();
}

void camera_move(vec3 mag, f32 dt)
{
    vec3 direction = vec3_create(0, 0, 0);
    direction = vec3_add(direction, vec3_scale(camera.right, mag.x));
    direction = vec3_add(direction, vec3_scale(Y_AXIS, mag.y));
    direction = vec3_add(direction, vec3_scale(camera.facing, mag.z));
    direction = vec3_scale(vec3_normalize(direction), camera.move_speed * dt);
    camera.position = vec3_add(camera.position, direction);
    update_view_matrix();
}

void camera_rotate(f32 mag, f32 dt)
{
    camera.yaw += mag * dt * camera.rotate_speed;
    update_orientation_vectors();
    update_view_matrix();
}

void camera_tilt(f32 mag, f32 dt)
{
    camera.pitch += mag * dt * camera.rotate_speed;
    if (camera.pitch > PI / 2 - EPSILON)
        camera.pitch = PI / 2 - EPSILON;
    if (camera.pitch < -PI / 2 + EPSILON)
        camera.pitch = -PI / 2 + EPSILON;
    update_orientation_vectors();
    update_view_matrix();
}

void camera_zoom(f32 mag, f32 dt)
{
    camera.zoom += mag * dt;
    update_proj_matrix();
}

vec3 camera_position(void)
{
    return camera.position;
}

bool camera_aabb_in_frustrum(AABB aabb)
{
    return intersect_aabb_hexahedron(aabb, camera.frustrum);
}

/* ------------------------- */

static void view(f32 m[16], vec3 r, vec3 u, vec3 f, vec3 p)
{
    f32 k1 = p.x * r.x + p.y * r.y + p.z * r.z;
    f32 k2 = p.x * u.x + p.y * u.y + p.z * u.z;
    f32 k3 = p.x * f.x + p.y * f.y + p.z * f.z;
    m[0]  = r.x; m[1]  = u.x; m[2]  = f.x; m[3]  = 0.0f;
    m[4]  = r.y; m[5]  = u.y; m[6]  = f.y; m[7]  = 0.0f;
    m[8]  = r.z; m[9]  = u.z; m[10] = f.z; m[11] = 0.0f;
    m[12] = -k1; m[13] = -k2; m[14] = -k3; m[15] = 1.0f;
}

static void orthographic(f32 m[16], f32 ar, f32 zoom)
{
    f32 r, l, t, b, f, n;
    b = -(t = zoom);
    l = -(r = ar * zoom);
    f = FAR_CLIP_DISTANCE;
    n = NEAR_CLIP_DISTANCE;
    f32 val1, val2, val3, val4, val5, val6;
    val1 = 2 / (r - l);
    val2 = 2 / (t - b);
    val3 = 2 / (f - n);
    val4 = -(r + l) / (r - l);
    val5 = -(t + b) / (t - b);
    val6 = -(f + n) / (f - n);
    m[0]  = val1; m[1]  = 0.0f; m[2]  = 0.0f; m[3]  = 0.0f;
    m[4]  = 0.0f; m[5]  = val2; m[6]  = 0.0f; m[7]  = 0.0f;
    m[8]  = 0.0f; m[9]  = 0.0f; m[10] = val3; m[11] = 0.0f;
    m[12] = val4; m[13] = val5; m[14] = val6; m[15] = 1.0f;
}

void perspective(f32 m[16], f32 ar, f32 fov)
{
    f32 a, b, c, d;
    f32 ncd = NEAR_CLIP_DISTANCE;
    f32 fcd = FAR_CLIP_DISTANCE;
    a = 1 / (ar * tan(fov / 2));
    b = 1 / (tan(fov / 2));
    c = (-ncd-fcd) / (ncd - fcd);
    d = (2 * fcd * ncd) / (ncd - fcd);
    m[0]  = a; m[1]  = 0; m[2]  = 0; m[3]  = 0;
    m[4]  = 0; m[5]  = b; m[6]  = 0; m[7]  = 0;
    m[8]  = 0; m[9]  = 0; m[10] = c; m[11] = 1;
    m[12] = 0; m[13] = 0; m[14] = d; m[15] = 0;
}