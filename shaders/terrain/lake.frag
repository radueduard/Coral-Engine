#version 460 core

layout (location = 0) in PerVertexData {
    vec4 position;
    vec3 normal;
    vec2 texCoords;
} f_in;

layout(location = 0) out vec4 FragColor;

void main() {
    vec3 position = f_in.position.xyz / f_in.position.w;
    FragColor = vec4(position, 1.0);
}