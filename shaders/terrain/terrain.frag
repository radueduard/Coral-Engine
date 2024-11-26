#version 460 core

layout (location = 0) in PerVertexData {
    vec4 position;
    vec3 normal;
    vec2 texCoords;
} f_in;

layout (set = 0, binding = 2) uniform sampler2DArray albedoTextures;

layout(location = 0) out vec4 FragColor;

vec4 GetTerrainColor(vec2 texCoords) {
    vec4 sand = texture(albedoTextures, vec3(texCoords, 0));
    vec4 grass = texture(albedoTextures, vec3(texCoords * 2, 1));
    vec4 rock = texture(albedoTextures, vec3(texCoords * 2, 2));
    vec4 snow = texture(albedoTextures, vec3(texCoords, 3));
    float height = f_in.position.y;

    vec3 N = normalize(f_in.normal);
    float slope = dot(N, vec3(0.0, 1.0, 0.0));
    slope *= slope;

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
    if (slope < 0.4) {
        heightColor = rock;
    } else if (slope < 0.7) {
        heightColor = mix(rock, heightColor, (slope - 0.4) / 0.3);
    }
    return heightColor;

}

void main() {
//    vec3 position = f_in.position.xyz / f_in.position.w;
//    FragColor = vec4(position, 1.0);
    FragColor = GetTerrainColor(f_in.texCoords);
}