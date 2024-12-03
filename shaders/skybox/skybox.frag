#version 460 core

layout (location = 0) in PerVertexData {
    vec4 position;
} v_in;

layout (set = 0, binding = 0) uniform samplerCube skybox;

layout (location = 0) out vec4 FragColor;

void main() {
    FragColor = texture(skybox, v_in.position.xyz / v_in.position.w);
}