#version 460 core

layout (location = 0) in PerVertexData {
    vec4 position;
    vec3 normal;
    vec2 texCoords;
} f_in;

layout (set = 0, binding = 1) uniform sampler2D albedo;
layout (set = 0, binding = 2) uniform sampler2D normal;

struct Light {
    vec4 position;
    vec4 speed;
    vec4 acceleration;
    vec4 color;
};

layout (set = 0, binding = 3) readonly buffer Lights {
    Light data[];
} lights;

struct Indices {
    uint data[16];
};

layout (set = 0, binding = 4, std430) readonly buffer LightIndices {
    Indices data[];
} lightIndices;

layout(location = 0) out vec4 FragColor;

vec3 DiffuseLighting(uint lightIndex) {
    Light light = lights.data[lightIndex];
    float distance = length(light.position.xyz - f_in.position.xyz);
    float attenuation = 1.0 / (1 + distance);
    vec3 direction = normalize(light.position.xyz - f_in.position.xyz);
    float diff = max(dot(f_in.normal, direction), 0.0);
    return vec3(light.color.rgb) / 16.0 * attenuation * diff;
}


void main() {
    if (f_in.position.y < -0.2) {
        discard;
    }

    uvec2 chunk = uvec2(floor(f_in.position.x + 15), floor(f_in.position.z + 15));

    vec4 lightColor = vec4(0.0, 0.0, 0.0, 1.0);
    Indices indices = lightIndices.data[chunk.x * 30 + chunk.y];
    for (int i = 0; i < 16; i++) {
        uint index = indices.data[i];
        if (index == 0xFFFFFFFF) {
            break;
        }
        vec3 light = DiffuseLighting(index);
        lightColor += vec4(light, 0.0);
    }
    vec4 albedoColor = texture(albedo, f_in.texCoords);
    FragColor = albedoColor * lightColor;
}