#version 460 core

layout (location = 0) in vec2 aPosition;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec4 aColor;

out vec2 TexCoord;
out vec4 Color;

void main()
{
    gl_Position = vec4(aPosition, 0.0f, 1.0f);
    TexCoord = aTexCoord;
    Color = aColor;
}