module wireframe;

struct VertexIn {
    float3 position : POSITION;
    float4 color : COLOR;
};

struct VertexOut {
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

struct Camera {
    float4x4 view;
    float4x4 projection;
    float4x4 inverseView;
    float4x4 inverseProjection;
}

[[vk::binding(0, 0)]]
ConstantBuffer<Camera> camera;

[[shader("vertex")]]
void main(VertexIn in, out VertexOut out, uniform float4x4 model) {
    float4x4 mvp = camera.projection * camera.view * model;
    out.position = mul(mvp, float4(in.position, 1.0));
    out.color = in.color;
}

[[shader("fragment")]]
void main(VertexOut in, out float4 color : SV_TARGET) {
    // Use the color from the vertex shader
    color = in.color;

    // Set the fragment to be transparent if the alpha is less than 0.5
    if (color.a < 0.5) {
        discard;
    }
}