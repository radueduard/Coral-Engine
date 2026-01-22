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
