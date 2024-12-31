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
    float chunk_positions[];
};

out vec2 UV;
out flat uint ID;

void main() {
    vec3 position = vec3(aInfo & 1, (aInfo >> 1) & 1, (aInfo >> 2) & 1);
    vec3 offset = vec3(aInstanceInfo & 31, (aInstanceInfo >> 5) & 31, (aInstanceInfo >> 10) & 31);
    vec3 chunk_offset = vec3(chunk_positions[3*gl_DrawID], chunk_positions[3*gl_DrawID+1], chunk_positions[3*gl_DrawID+2]);
    UV = vec2((aInfo >> 3) & 1, (aInfo >> 4) & 1);
    ID = (aInstanceInfo >> 15);
    gl_Position = proj * view * vec4(position + offset + chunk_offset, 1.0);
}