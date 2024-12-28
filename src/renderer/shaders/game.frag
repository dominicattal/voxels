#version 460

out vec4 FragColor;

uniform sampler2D textures[16];

struct Tex 
{
    vec4 uv;
    int location;
};

layout (std430, binding = 1) readonly buffer Textures
{
    Tex texs[];
};

in vec2 UV;
in flat int location;

void main() {
    FragColor = texture(textures[location], UV);
}