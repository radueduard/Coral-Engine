#version 460 core

layout (location = 0) in PerVertexData {
    vec4 position;
    vec4 ndcPosition;
    vec3 normal;
    vec2 texCoords;
} f_in;

layout (set = 0, binding = 0) uniform Camera {
    mat4 view;
    mat4 projection;
    mat4 inverseView;
    mat4 inverseProjection;
    mat4 flippedView;
    mat4 inverseFlippedView;
} camera;

layout (set = 1, binding = 0) uniform sampler2D reflectionTexture;

struct Light {
    vec4 origin;    // vec2
    vec4 position;  // vec3, w = radius
    vec4 color;
};

layout (set = 1, binding = 1) readonly buffer Lights {
    Light data[];
} lights;

struct Indices {
    uint data[64];
};

layout (set = 1, binding = 2, std430) readonly buffer LightIndices {
    Indices data[];
} lightIndices;

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
    float diff = max(dot(normal, lightDir), 0.0);
    return light.color.rgb * diff * attenuation / 64.0;
}

void main() {
    vec4 worldPos = f_in.position;
    uvec2 chunk = uvec2((f_in.ndcPosition.xy / f_in.ndcPosition.w + 1.0) / 2.0 * vec2(64, 64));

    vec4 reflectionPosition = vec4(camera.projection * camera.flippedView * worldPos);
    reflectionPosition = reflectionPosition / reflectionPosition.w;
    reflectionPosition = 0.5 * reflectionPosition + 0.5;
    vec4 reflectionColor = texture(reflectionTexture, reflectionPosition.xy);

    vec4 lightColor = vec4(0.0, 0.0, 0.0, 1.0);
    Indices indices = lightIndices.data[chunk.x * 64 + chunk.y];
    for (int i = 1; i < indices.data[0]; i++) {
        lightColor += vec4(DiffuseLighting(indices.data[i]), 0.0);
    }

    FragColor = reflectionColor * 0.7 + lightColor;
    FragColor.a = 1.0;
}
