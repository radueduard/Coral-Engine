module grid;

import camera;

layout (set = 0) ConstantBuffer<Camera> camera;

const static float3[4] pos = {
    float3(-1.0, 0.0, -1.0),
    float3( 1.0, 0.0, -1.0),
    float3( 1.0, 0.0, 1.0),
    float3(-1.0, 0.0, 1.0)
};
const static int[6] indices = { 0, 1, 2, 2, 3, 0 };

struct PipelineData {
    float4 position : SV_Position;
    float2 uv;
    float2 camPos;
};

struct PushConstants {
    float4x4 modelMatrix;
    float4 origin;
};

[shader("vertex")]
PipelineData vertex(
    uint vertexID : SV_VertexID,
    uniform PushConstants pc
) {
    // Get camera world position from inverse view matrix
    // For column-major matrices, translation is in the 4th column (index [0][3], [1][3], [2][3])
    float3 cameraPos = float3(camera.inverseView[0][3], camera.inverseView[1][3], camera.inverseView[2][3]);
    float3 position = pos[indices[vertexID]] * 1000.f;
    // Position the grid on the XZ plane centered at the camera's XZ position
    position.xz += cameraPos.xz;
    position += pc.origin.xyz;

    float4x4 MVP = mul(camera.projection, camera.view);

    return {
        mul(MVP, float4(position, 1.0)),
        position.xz,
        cameraPos.xz
    };
}

float log10(float x) {
    return log(x) / log(10.0);
}

float satf(float x) {
    return clamp(x, 0.0, 1.0);
}

float2 satv(float2 x) {
    return clamp(x, float2(0.0), float2(1.0));
}

float max2(float2 x) {
    return max(x.x, x.y);
}

float2 mod(float2 x, float y) {
    return x - y * floor(x / y);
}

float4 gridColor(float2 uv, float2 camPos) {
    struct GridParams {
        float size = 1000.0;
        float cellSize = 0.025f;
        float4 colorThin = { 0.5f, 0.5f, 0.5f, 1.0f };
        float4 colorThick = { 1.0f, 1.0f, 1.0f, 1.0f };
        float minPixelBetweenCells = 2.0f;
    } gridParams = {};

    float2 dudv = float2(length(float2(ddx(uv.x), ddy(uv.x))), length(float2(ddx(uv.y), ddy(uv.y))));
    float lodLevel = max(0.0, log10((length(dudv) * gridParams.minPixelBetweenCells) / gridParams.cellSize) + 1.0);
    float lodFade = fract(lodLevel);
    float lod0 = gridParams.cellSize * pow(10.0, floor(lodLevel));
    float lod1 = lod0 * 10.0;
    float lod2 = lod1 * 10.0;
    dudv *= 4.0;
    uv += dudv * 0.5;
    float lod0a = max2(float2(1.0) - abs(satv(mod(uv, lod0) / dudv) * 2.0 - float2(1.0)));
    float lod1a = max2(float2(1.0) - abs(satv(mod(uv, lod1) / dudv) * 2.0 - float2(1.0)));
    float lod2a = max2(float2(1.0) - abs(satv(mod(uv, lod2) / dudv) * 2.0 - float2(1.0)));
    
    float lineAlpha = lod2a > 0.0 ? lod2a : lod1a > 0.0 ? lod1a : (lod0a * (1.0 - lodFade));
    
    int chosenLod = lod2a > 0.0 ? 2 : lod1a > 0.0 ? 1 : 0;

    float4 c = lod2a > 0.0 ? gridParams.colorThick 
             : lod1a > 0.0 ? lerp(gridParams.colorThick, gridParams.colorThin, lodFade) 
                           : gridParams.colorThin;

    float2 centeredUV = uv - camPos;
    float opacityFalloff = (1.0 - satf(length(centeredUV) / gridParams.size));
    c.a *= lineAlpha * opacityFalloff;
    
    switch (chosenLod) {
        case 0:
            if (c.a <= 0.5) discard;
        case 1:
            if (c.a <= 0.0) discard;
        case 2:
            if (c.a <= 0.0) discard;
    }
    
    return c;
}

[shader("fragment")]
float4 fragment(
    PipelineData in
) : SV_Target {
    return gridColor(in.uv, in.camPos);
}