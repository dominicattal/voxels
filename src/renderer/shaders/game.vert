#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aUV;
layout (location = 2) in vec3 aPosOffset;
layout (location = 3) in float aID;

layout (std140) uniform Matrices
{
    mat4 view;
    mat4 proj;
};

out vec2 UV;
out flat int ID;

void main() {
    gl_Position = proj * view * vec4(aPos + aPosOffset, 1.0);
    UV = aUV;
    ID = int(round(aID));
}