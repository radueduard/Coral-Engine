#version 460 core

#pragma Position
layout(location = 0) in vec3 inPosition;

#pragma Normal
layout(location = 1) in vec3 inNormal;

#pragma Tangent
layout(location = 2) in vec4 inTangent;

#pragma TexCoord0
layout(location = 3) in vec2 inTexCoord0;

layout(set = 0, binding = 0) uniform Camera {
    mat4 view;
    mat4 projection;
    mat4 inverseView;
    mat4 inverseProjection;
} camera;

layout(push_constant) uniform PushConstants {
    mat4 model;
} push;

layout(location = 0) out PerVertex {
    vec3 position;
    vec3 normal;
    vec3 tangent;
    vec3 bitangent;
    vec2 texCoord;
    vec4 color;
} v_out;

void main() {
    v_out.color = vec4(inNormal, 1.0);
    v_out.position = (push.model * vec4(inPosition, 1.0)).xyz;
    v_out.normal = normalize((push.model * vec4(inNormal, 0.0)).xyz);
    v_out.tangent = normalize((push.model * vec4(inTangent.xyz, 0.0)).xyz);
    v_out.bitangent = normalize(cross(v_out.normal, v_out.tangent) * inTangent.w);
    v_out.texCoord = inTexCoord0;
    gl_Position = camera.projection * camera.view * push.model * vec4(inPosition, 1.0);
}