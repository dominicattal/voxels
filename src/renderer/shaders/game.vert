#version 460 core

layout (location = 0) in uint aInfo;
layout (location = 1) in uint aInstanceInfo;

layout (std140) uniform Matrices
{
    mat4 view;
    mat4 proj;
};

layout (std430, binding = 2) readonly buffer ChunkPositions
{
    int chunk_positions[];
};

#define NEGX 0
#define POSX 1
#define NEGY 2
#define POSY 3
#define NEGZ 4
#define POSZ 5

out vec2 UV;
out flat uint ID;
out flat vec3 normal;

vec3 normals[6] = {
    vec3(-1, 0, 0),
    vec3(1, 0, 0),
    vec3(0, -1, 0),
    vec3(0, 1, 0),
    vec3(0, 0, -1),
    vec3(0, 0, 1)
};

void main() {
    uint width = 1 + ((aInstanceInfo >> 15) & 31);
    uint height = 1 + ((aInstanceInfo >> 20) & 31);
    uint a = width * (aInfo & 1);
    uint b = height * ((aInfo >> 1) & 1);
    int face = chunk_positions[4*gl_DrawID+3];
    normal = normals[face];
    vec3 position;
    switch (face) {
       case NEGX:
            position = vec3(0, a, b);
            UV = vec2(b, width-a);
            break;
        case POSX:
            position = vec3(1, b, a);
            UV = vec2(a, height-b);
            break;
        case NEGY:
            position = vec3(b, 0, a);
            UV = vec2(a, b);
            break;
        case POSY:
            position = vec3(a, 1, b);
            UV = vec2(a, b);
            break;
        case NEGZ:
            position = vec3(a, b, 0);
            UV = vec2(a, height-b);
            break;
        case POSZ:
            position = vec3(b, a, 1);
            UV = vec2(b, width-a);
            break;
    }
    vec3 offset = vec3(aInstanceInfo & 31, (aInstanceInfo >> 5) & 31, (aInstanceInfo >> 10) & 31);
    vec3 chunk_offset = vec3(chunk_positions[4*gl_DrawID], chunk_positions[4*gl_DrawID+1], chunk_positions[4*gl_DrawID+2]);
    ID = (aInstanceInfo >> 25);
    gl_Position = proj * view * vec4(position + offset + chunk_offset, 1.0);
}