module depth;

import vertex;
import camera;

[shader("vertex")]
public void vertex(
    in Vertex input,
    ConstantBuffer<Camera> camera,
    uniform float4x4 modelMatrix,
    out float4 position : SV_Position,
) {
    float4 worldPosition = mul(modelMatrix, float4(input.position, 1.0));
    position = mul(camera.projection, mul(camera.view, worldPosition));
}