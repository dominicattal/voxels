#version 460 core

out vec4 FragColor;

in vec2 TexCoord;
in vec4 Color;
in flat float Id;

uniform sampler2D ourTexture;

void main()
{
    vec4 texColor = texture(ourTexture, TexCoord);
    FragColor = mix(Color, texColor, Id);
}