#version 460

out vec4 FragColor;

uniform sampler2D textures[16];

in vec2 UV;
in flat int location;

void main() {
    FragColor = texture(textures[location], UV);
    FragColor = vec4(1.0, 0.0, 1.0, 1.0);
}