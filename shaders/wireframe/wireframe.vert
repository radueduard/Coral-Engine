#version 460 core

#pragma Position
layout(location = 0) in vec3 inPosition;

#pragma Normal
layout(location = 1) in vec3 inNormal;

layout(set = 0, binding = 0) uniform Camera {
    mat4 view;
    mat4 projection;
    mat4 inverseView;
    mat4 inverseProjection;
} camera;

layout(push_constant) uniform PushConstants {
    mat4 model;
} push;

layout(location = 0) out PerVertexData {
    vec4 color;
} v_out;

void main() {
    v_out.color = vec4(1.0, 1.0, 1.0, 1.0);
    gl_Position = camera.projection * camera.view * push.model * vec4(inPosition, 1.0);
}