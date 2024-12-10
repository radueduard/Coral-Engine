#version 460 core

layout(location = 0) in vec3 inPosition;

layout(location = 0) out vec3 outColor;

layout(set = 0, binding = 0) uniform Camera {
    mat4 view;
    mat4 projection;
    mat4 inverseView;
    mat4 inverseProjection;
    mat4 flippedView;
    mat4 inverseFlippedView;
} camera;

void main() {
    gl_Position = camera.projection * camera.view * vec4(inPosition, 1.0);
    outColor = vec3(1.0);
}