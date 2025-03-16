#version 450 core

layout(location = 0) in FragmentData {
    vec3 position;
    vec3 normal;
    vec2 texCoord;
} fragmentData;

layout(location = 0) out vec4 color;

void main() {
    color = vec4(1.0, 0.0, 0.0, 1.0);
}