#version 460 core

layout(location = 0) in vec3 inPosition;

layout(set = 0, binding = 0) uniform Camera {
    mat4 view;
    mat4 projection;
    mat4 inverseView;
    mat4 inverseProjection;
    mat4 flippedView;
    mat4 inverseFlippedView;
} camera;

layout(push_constant) uniform PushConstants {
    mat4 model;
} push;

void main() {
    gl_Position = camera.projection * camera.view * push.model * vec4(inPosition, 1.0);
}