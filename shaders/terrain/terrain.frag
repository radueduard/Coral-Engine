#version 460 core

layout (location = 0) in PerVertexData {
    vec4 position;
    vec4 ndcPosition;
    vec3 normal;
    vec2 texCoords;
} f_in;

layout (set = 1, binding = 1) uniform sampler2D albedo;
layout (set = 1, binding = 2) uniform sampler2D normal;

struct Light {
    vec4 origin;    // vec2
    vec4 position;  // vec3, w = radius
    vec4 color;
};

layout (set = 1, binding = 3) readonly buffer Lights {
    Light data[];
} lights;

struct Indices {
    uint data[64];
};

layout (set = 1, binding = 4, std430) readonly buffer LightIndices {
    Indices data[];
} lightIndices;

layout(push_constant) uniform Settings {
    uint flipped;
} settings;

layout(location = 0) out vec4 FragColor;

vec3 DiffuseLighting(uint lightIndex) {
    Light light = lights.data[lightIndex];
    float distance = length(light.position.xyz - f_in.position.xyz);
    float radius = light.position.w;
    if (distance > radius) {
        return vec3(0.0);
    }
    float attenuation = 1.0 - distance / radius;
    vec3 normal = normalize(f_in.normal);
    vec3 lightDir = normalize(light.position.xyz - f_in.position.xyz);
    float diff = max(dot(normal, lightDir), 0.0) * 0.25;
    return light.color.rgb * diff * attenuation;
}


void main() {
    if (f_in.position.y < 0) {
        discard;
    }

    vec2 ndc = (f_in.ndcPosition.xy / f_in.ndcPosition.w + 1.0) / 2.0;
    uvec2 chunk = uvec2(ndc * vec2(64, 64));
    vec4 lightColor = vec4(0.0, 0.0, 0.0, 1.0);
    Indices indices = lightIndices.data[chunk.x * 64 + chunk.y];
    for (int i = 1; i < indices.data[0]; i++) {
        lightColor += vec4(DiffuseLighting(indices.data[i]), 0.0);
    }

    vec4 albedoColor = texture(albedo, f_in.texCoords);
    FragColor = albedoColor * lightColor;
}