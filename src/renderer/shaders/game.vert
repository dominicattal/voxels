#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aUV;
layout (location = 2) in vec3 aPosOffset;
layout (location = 3) in vec4 aUVOffset;
layout (location = 4) in float aLocation;

uniform mat4 proj;
uniform mat4 view;

out vec2 UV;
out flat int location;

void main() {
    gl_Position = proj * view * vec4(aPos + aPosOffset, 1.0);
    UV.x = aUVOffset.x + aUV.x * aUVOffset.w;
    UV.y = aUVOffset.y + aUV.y * aUVOffset.z;
    location = int(round(aLocation));
}