#version 460 core

layout (location = 0) in PerVertexData {
    vec4 position;
    vec3 normal;
    vec2 texCoords;
} f_in;

layout (set = 0, binding = 1) uniform sampler2DArray albedoTextures;

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

vec4 GetTerrainColor(vec2 texCoords) {
    float height = f_in.position.y;
    if (height < -.1) {
        discard;
    }

    vec4 sand = texture(albedoTextures, vec3(texCoords, 0));
    vec4 grass = texture(albedoTextures, vec3(texCoords * 2, 1));
    vec4 rock = texture(albedoTextures, vec3(texCoords * 2, 2));
    vec4 snow = texture(albedoTextures, vec3(texCoords, 3));

    vec3 N = normalize(f_in.normal);
    float slope = dot(N, vec3(0.0, 1.0, 0.0));

    vec4 heightColor = vec4(0.0);
    if (height < 0.1) {
        heightColor = sand;
    } else if (height > 0.1 && height < 0.3) {
        heightColor = mix(sand, grass, (height - 0.1) / 0.2);
    } else if (height > 0.3 && height < 6) {
        heightColor = grass;
    } else if (height > 6 && height < 8) {
        heightColor = mix(grass, snow, (height - 6) / 2);
    } else {
        heightColor = snow;
    }
    if (slope < 0.5 && slope > 0.1) {
        heightColor = mix(rock, heightColor, (slope - 0.1) / 0.4);
    } else if (slope < 0.1) {
        heightColor = rock;
    }
    return heightColor;
}

vec3 DiffuseLighting(uint lightIndex) {
    Light light = lights.data[lightIndex];
    float distance = length(light.position.xyz - f_in.position.xyz);
    float attenuation = 1.0 / (1 + distance);
    vec3 direction = normalize(light.position.xyz - f_in.position.xyz);
    float diff = max(dot(f_in.normal, direction), 0.0);
    return vec3(light.color.rgb) / 16.0 * attenuation * diff;
}


void main() {
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

    FragColor = GetTerrainColor(f_in.texCoords) * lightColor;
//    FragColor = lightColor;
}