#version 460

out vec4 FragColor;

uniform sampler2D textures[2];

in vec2 UV;

void main() {
    FragColor = texture(textures[1], UV);
}