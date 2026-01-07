module depth;

import vertex;
import camera;

[shader("vertex")]
float4 vertex(
    Vertex input,
    ConstantBuffer<Camera> camera,
    uniform float4x4 modelMatrix,
) : SV_Position {
    float4 worldPosition = mul(modelMatrix, float4(input.position, 1.0));
    return mul(camera.projection, mul(camera.view, worldPosition));
}

[shader("pixel")]
float4 fragment(float4 position : SV_Position) : SV_Target {
    float depth = position.z / position.w;
    depth = depth * 0.5 + 0.5;
    return float4(depth, depth, depth, 1.0);
}