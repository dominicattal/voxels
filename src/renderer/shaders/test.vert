#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aUV;
layout (location = 2) in float aLocation;

uniform mat4 proj;
uniform mat4 view;

out vec2 UV;
out flat int location;

void main() {
    gl_Position = proj * view * vec4(aPos, 1.0);
    UV = aUV;
    location = int(round(aLocation));
}