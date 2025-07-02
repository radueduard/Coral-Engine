#version 460 core

layout(location = 0) in PerVertex {
    vec3 position;
    vec3 normal;
    vec3 tangent;
    vec3 bitangent;
    vec2 texCoord;
    vec4 color;
} v_in;

layout(location = 0) out vec4 FragColor;

layout(set = 0, binding = 0) uniform Camera {
    mat4 view;
    mat4 projection;
    mat4 inverseView;
    mat4 inverseProjection;
} camera;

layout(set = 0, binding = 1) uniform LightCounts {
    uvec3 counts; // x = point lights, y = directional lights, z = spot lights
} lightCounts;

struct PointLight {
    vec3 position;
    vec3 color;
    vec3 attenuation; // x = constant, y = linear, z = quadratic
    float radius;
};

layout(set = 0, binding = 2) readonly buffer PointLights {
    PointLight data[];
} pointLights;

struct DirectionalLight {
    vec3 direction;
    vec3 color;
    float intensity;
};

layout(set = 0, binding = 3) readonly buffer DirectionalLights {
    DirectionalLight data[];
} directionalLights;

struct SpotLight {
    vec3 position;
    vec3 direction;
    vec3 color;
    float innerAngle; // in radians
    float outerAngle; // in radians
    float intensity;
    float radius;
};

layout(set = 0, binding = 4) readonly buffer SpotLights {
    SpotLight data[];
} spotLights;


layout(set = 1, binding = 0) uniform Material {
    float alphaCutoff;
    uint doubleSided;
    float roughnessFactor;
    float metallicFactor;
    vec3 emissiveFactor;
    vec4 baseColorFactor;
} material;

layout(set = 1, binding = 1) uniform sampler2D albedoTexture;
layout(set = 1, binding = 2) uniform sampler2D normalTexture;
layout(set = 1, binding = 3) uniform sampler2D metallicTexture;
layout(set = 1, binding = 4) uniform sampler2D roughnessTexture;
layout(set = 1, binding = 5) uniform sampler2D emissiveTexture;
layout(set = 1, binding = 6) uniform sampler2D ambientOcclusionTexture;

vec3 GetNormal() {
    vec3 normal = texture(normalTexture, v_in.texCoord).xyz * 2.0 - 1.0;
    mat4 tbn = mat4(
        vec4(v_in.tangent, 0.0),
        vec4(v_in.bitangent, 0.0),
        vec4(v_in.normal, 0.0),
        vec4(0.0, 0.0, 0.0, 1.0)
    );
    normal = normalize(tbn * vec4(normal, 0.0)).xyz;
    return normal;
}

void main() {
    FragColor = vec4(GetNormal() * 0.5 + 0.5, 1.0); // Output the normal in RGB format
//    FragColor = vec4(texture(emissiveTexture, v_in.texCoord).rgb, 1.0); // Output the emissive color
}