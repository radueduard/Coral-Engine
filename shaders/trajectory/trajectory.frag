#version 460 core

layout (location = 0) in PerVertexData {
    vec4 color;
} v_in;

layout(location = 0) out vec4 FragColor;

void main() {
    FragColor = vec4(v_in.color.rgb, 1.0);
}