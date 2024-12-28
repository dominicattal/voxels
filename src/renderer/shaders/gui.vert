#version 460 core

layout (location = 0) in vec2 aPosition;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec4 aColor;
layout (location = 3) in float aID;

out vec2 UV;
out vec4 Color;
out flat int ID;

void main()
{
    gl_Position = vec4(aPosition, 0.0f, 1.0f);
    UV = aTexCoord;
    Color = aColor;
    ID = int(round(aID));
}