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

void main() {
    vec3 position = vec3(aInfo & 1, 0, (aInfo >> 1) & 1);
    switch (chunk_positions[4*gl_DrawID+3]) {
        case NEGX:
            position.xyz = position.xzy;
            break;
        case POSX:
            position.xyz = position.yzx;
            position.x++;
            break;
        case NEGY:
            position.xyz = position.zyx;
            break;
        case POSY:
            position.y++;
            break;
        case NEGZ:
            position.xyz = position.yxz;
            break;
        case POSZ:
            position.xyz = position.zxy;
            position.z++;
            break;
    }
    vec3 offset = vec3(aInstanceInfo & 31, (aInstanceInfo >> 5) & 31, (aInstanceInfo >> 10) & 31);
    vec3 chunk_offset = vec3(chunk_positions[4*gl_DrawID], chunk_positions[4*gl_DrawID+1], chunk_positions[4*gl_DrawID+2]);
    UV = vec2(aInfo & 1, (aInfo >> 1) & 1);
    ID = (aInstanceInfo >> 15);
    gl_Position = proj * view * vec4(position + offset + chunk_offset, 1.0);
}