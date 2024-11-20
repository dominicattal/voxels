#version 460 core

#extension GL_ARB_bindless_texture : require

out vec4 FragColor;

layout (binding = 0, std430) readonly buffer ssbo
{
    uvec2 tex[];
};

in vec2 TexCoord;
in vec4 Color;
in flat int TexId;

void main()
{
    vec4 tex = texture(sampler2D(tex[TexId]), TexCoord);
    if (TexId == 2)
        tex = vec4(0.0f, 0.0f, 0.0f, tex.r);
    FragColor = tex * Color;
}