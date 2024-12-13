#version 460 core

out vec4 FragColor;

layout (location = 0) uniform sampler2D Texture;

in vec2 TexCoord;
in vec4 Color;
in flat int TexId;

void main()
{
    FragColor = Color;
    if (TexId != 1) {
        vec4 tex = texture(Texture, TexCoord);
        tex = vec4(1.0f, 1.0f, 1.0f, tex.r);
        FragColor = tex * Color;
    }
}