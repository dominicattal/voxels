#version 460 core

out vec4 FragColor;

uniform sampler2D textures[16];

struct Tex {
    float u;
    float v;
    float w;
    float h;
    int location;
};

layout (std430, binding = 1) readonly buffer Textures
{
    Tex texs[];
};

in vec2 UV;
in vec4 Color;
in flat int ID;

void main()
{
    vec2 TexCoord;
    Tex tex = texs[ID];
    TexCoord.x = tex.u + UV.x * tex.w;
    TexCoord.y = tex.v + UV.y * tex.h;
    FragColor = texture(textures[tex.location], TexCoord) * Color;
}