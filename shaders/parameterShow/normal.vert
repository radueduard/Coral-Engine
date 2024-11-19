#version 460 core

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec4 inTangent;
layout(location = 3) in vec2 inTexCoords0;
layout(location = 4) in vec2 inTexCoords1;
layout(location = 5) in vec4 inColor;

layout(set = 0, binding = 0) readonly buffer Models {
    mat4 get[4096];
} models;

layout(set = 0, binding = 1) readonly buffer MVPs {
    mat4 get[4096];
} mvps;

layout(push_constant) uniform UBO {
    uint index;
} PerMeshData;

layout(location = 0) out PerVertexData
{
    vec2 uv0;
    vec2 uv1;
    mat3 TBN;
} v_out;

void main()
{
    gl_Position = mvps.get[PerMeshData.index] * vec4(inPosition, 1.0);

    vec3 N = normalize(mat3(transpose(inverse(models.get[PerMeshData.index]))) * inNormal);
    vec3 T = normalize(mat3(transpose(inverse(models.get[PerMeshData.index]))) * inTangent.xyz);
    vec3 B = cross(N, T) * inTangent.w;

    v_out.uv0 = inTexCoords0;
    v_out.uv1 = inTexCoords1;
    v_out.TBN = mat3(T, B, N);
}