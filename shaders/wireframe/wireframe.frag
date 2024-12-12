#version 460 core

layout(location = 0) in PerVertex {
    vec4 color;
} v_in;

layout(location = 0) out vec4 FragColor;

void main() {
    FragColor = v_in.color;
}