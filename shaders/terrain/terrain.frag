#version 460 core

layout (location = 0) in PerVertexData {
    vec4 position;
    vec3 normal;
    vec2 texCoords;
} f_in;

layout (set = 0, binding = 1) uniform sampler2DArray albedoTextures;

layout(location = 0) out vec4 FragColor;

vec4 GetTerrainColor(vec2 texCoords) {
    vec4 sand = texture(albedoTextures, vec3(texCoords, 0));
    vec4 grass = texture(albedoTextures, vec3(texCoords, 1));
    vec4 rock = texture(albedoTextures, vec3(texCoords, 2));
    vec4 snow = texture(albedoTextures, vec3(texCoords, 3));
    float height = f_in.position.y;

    vec3 N = normalize(f_in.normal);
    float slope = abs(dot(N, vec3(0.0, 1.0, 0.0)));

    if (height < 0.0) {
        return sand;
    } else if (height < 0.1) {
        if (slope < 0.5) {
            return mix(sand, grass, height * 10.0);
        } else {
            return mix(sand, rock, height * 10.0);
        }
    } else if (height < 4.1) {
        if (slope < 0.5) {
            return mix(grass, rock, (height - 0.1) / 2.0);
        }else {
            return mix(grass, snow, (height - 0.1) / 2.0);
        }
    } else {
        if (slope < 0.5) {
            return mix(rock, snow, (height - 0.1) / 4.0);
        } else {
            return snow;
        }
    }
}

void main() {
//    vec3 position = f_in.position.xyz / f_in.position.w;
//    FragColor = vec4(position, 1.0);
    FragColor = GetTerrainColor(f_in.texCoords);
}