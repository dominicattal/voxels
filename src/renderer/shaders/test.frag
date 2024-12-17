#version 460

out vec4 FragColor;

layout (location = 1) uniform sampler2D texture;

in vec2 UV;

void main() {
    FragColor = texture(texture, UV);
}