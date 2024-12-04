#version 460 core

layout (location = 0) in PerVertexData {
    vec4 position;
    vec3 normal;
    vec2 texCoords;
} f_in;

layout (set = 0, binding = 0) uniform sampler2D reflectionTexture;

layout (set = 0, binding = 1) uniform CameraData {
    mat4 view;
    mat4 projection;
    mat4 inverseView;
    mat4 inverseProjection;
    mat4 flippedView;
    mat4 flippedInverseView;
} cameraData;

struct Light {
    vec4 position;
    vec4 speed;
    vec4 acceleration;
    vec4 color;
};

layout (set = 0, binding = 2) readonly buffer Lights {
    Light data[];
} lights;

struct Indices {
    uint data[16];
};

layout (set = 0, binding = 3, std430) readonly buffer LightIndices {
    Indices data[];
} lightIndices;


layout(location = 0) out vec4 FragColor;

vec3 DiffuseLighting(uint lightIndex) {
    Light light = lights.data[lightIndex];
    float distance = length(light.position.xyz - f_in.position.xyz);
    float diff = max(dot(normalize(f_in.normal), normalize(light.position.xyz - f_in.position.xyz)), 0.0);
    float attenuation = 1.0 / (1 + distance);
    return vec3(light.color.rgb) / 16.0 * attenuation * diff;
}

void main() {
    vec4 worldPos = f_in.position;
    uvec2 chunk = uvec2(floor(f_in.position.x + 15), floor(f_in.position.z + 15));

    vec4 reflectionPosition = cameraData.projection * cameraData.flippedView * worldPos;
    reflectionPosition.z = 0.0;
    reflectionPosition = reflectionPosition / reflectionPosition.w;
    reflectionPosition = 0.5 * reflectionPosition + 0.5;
    vec4 reflectionColor = texture(reflectionTexture, reflectionPosition.xy);


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
    FragColor = reflectionColor * 0.7 + lightColor;
    FragColor.a = 1.0;
}
