#version 460 core

out vec4 FragColor;

in vec2 TexCoord;
in vec4 Color;
in flat float UseTexture;

uniform sampler2D ourTexture;

void main()
{
    vec4 texColorRed = texture(ourTexture, TexCoord);
    vec4 texColor = vec4(0, 0, 0, texColorRed.x);
    FragColor = Color;
    if (UseTexture > 0)
        FragColor *= texColor;
}