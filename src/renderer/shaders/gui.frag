#version 460 core

out vec4 FragColor;

uniform sampler2D textures[16];

uniform int TEX_ID_FONT;

struct Tex 
{
    vec4 uv;
    int location;
};

layout (std430, binding = 1) buffer readonly Textures
{
    Tex texs[];
};

in vec2 TexCoord;
in vec4 Color;
in flat int TexId;

void main()
{
    FragColor = Color;
    if (TexId == TEX_ID_FONT) {
        vec4 tex = texture(textures[0], TexCoord);
        tex = vec4(1.0f, 1.0f, 1.0f, tex.r);
        FragColor = tex * Color;
    }
}