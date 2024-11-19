#version 460 core

layout (location = 0) in PerVertexData {
    vec2 uv0;
    vec2 uv1;
    mat3 TBN;
} f_in;

layout(push_constant) uniform Material {
    layout(offset = 4) float alpha_cutoff;
    uint albedo_texture_index;
    uint normal_texture_index;
} material;

layout(set = 0, binding = 2) uniform sampler2DArray albedoTextureArray;
layout(set = 0, binding = 3) uniform sampler2DArray normalTextureArray;
layout(set = 0, binding = 4) uniform sampler2DArray metallicRoughnessTextureArray;
//layout(set = 0, binding = 5) uniform sampler2DArray occlusionTextureArray;
//layout(set = 0, binding = 6) uniform sampler2DArray emissiveTextureArray;

layout(location = 0) out vec4 FragColor;

void main() {
    vec4 albedo = texture(albedoTextureArray, vec3(f_in.uv0, float(material.albedo_texture_index)));
    if (albedo.a < material.alpha_cutoff) {
        discard;
    }

    if (material.normal_texture_index == 0xFFFFFFFF) {
        discard;
    }

    vec3 normalTex = texture(normalTextureArray, vec3(f_in.uv0, float(material.normal_texture_index))).rgb;
    normalTex = normalize(normalTex * 2.0 - 1.0);
    vec3 normal = normalize(f_in.TBN * normalTex);

    FragColor = vec4(normal, 1.0);

//    FragColor = vec4(albedo.rgb, 1.0);
}