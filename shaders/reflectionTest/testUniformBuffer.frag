#version 450 core

struct Ceva {
    ivec4 ceva;
    vec4 ceva2;
};

layout (set = 0, binding = 0) uniform UniformBufferObject {
    vec4 color;
    float time;
    Ceva ceva[4];
} ubo;

layout(location = 0) out vec4 fragColor;

void main() {
    fragColor = ubo.color;
}