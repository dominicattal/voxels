#version 460

out vec4 FragColor;

uniform sampler2D textures[16];

struct Tex {
    float u;
    float v;
    float w;
    float h;
    int location;
};

layout (std430, binding = 1) readonly buffer Textures
{
    Tex texs[];
};

in vec2 UV;
in flat uint ID;
in flat vec3 normal;

void main() {
    vec2 TexCoord;
    Tex tex = texs[ID];
    TexCoord.x = tex.u + (UV.x - int(UV.x)) * tex.w;
    TexCoord.y = tex.v + (UV.y - int(UV.y)) * tex.h;

    float ambient_strength = 0.6;
    vec3 light_dir = normalize(vec3(1,2,3));
    vec3 light_color = vec3(1.0f, 1.0f, 1.0f);
    vec3 ambient = ambient_strength * light_color;
    float diff = max(dot(normal, light_dir), 0);
    vec3 diffuse = diff * light_color;
    vec3 result = (ambient + diffuse);

    FragColor = texture(textures[tex.location], TexCoord) * vec4(result, 1.0);
}
