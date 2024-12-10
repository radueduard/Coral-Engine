#version 460 core

layout(location = 0) in vec3 inPosition;

layout(location = 0) out vec3 color;

struct Particle {
    vec4 position;
    vec4 velocity;
    vec4 acceleration;
    vec4 color;
};

layout (set = 0, binding = 0) uniform Camera {
    mat4 view;
    mat4 projection;
    mat4 inverseView;
    mat4 inverseProjection;
    mat4 flippedView;
    mat4 inverseFlippedView;
} camera;

layout(set = 1, binding = 0) readonly buffer Particles {
    Particle data[];
} particles;

layout(push_constant) uniform Settings {
    uint flipped;
} settings;

void main() {
    Particle particle = particles.data[gl_InstanceIndex];

    mat4 model = mat4(0.01);
    model[3] = vec4(particle.position.xyz, 1.0);

    mat4 view = settings.flipped == 1 ? camera.flippedView : camera.view;
    gl_Position = camera.projection * view * model * vec4(inPosition, 1.0);
    color = particle.color.rgb;
}