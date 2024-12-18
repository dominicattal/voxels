#version 460 core

out vec4 FragColor;

uniform sampler2D textures[2];

uniform int TEX_ID_FONT;

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