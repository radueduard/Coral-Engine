#version 450 core

#pragma Postion
layout(location = 0) in vec3 position;

#pragma Normal
layout(location = 1) in vec3 normal;

#pragma TexCoord0
layout(location = 3) in vec2 texCoord0;

#pragma Camera
layout(set = 0, binding = 0) uniform Camera
{
    mat4 view;
    mat4 projection;
    mat4 viewProjection;
    mat4 inverseView;
    mat4 inverseProjection;
};

#pragma Model
layout(set = 1, binding = 0) uniform Model
{
    mat4 model;
} model;

layout(location = 0) out VertexData
{
    vec3 position;
    vec3 normal;
    vec2 texCoord0;
} vertexData;

void main()
{
    vec3 position = vec3(model * vec4(position, 1.0));
    vertexData.position = position;
    vertexData.normal = mat3(transpose(inverse(model))) * normal;
    vertexData.texCoord0 = texCoord0;
    gl_Position = viewProjection * vec4(position, 1.0);
}