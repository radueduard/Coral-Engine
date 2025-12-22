module rasterizer;

struct Vertex {
    float3 position;
    float3 color;
};

struct Triangle {
    Vertex p1;
    Vertex p2;
    Vertex p3;
};

struct Camera {
    float4x4 view;
    float4x4 projection;
    float4x4 inverseView;
    float4x4 inverseProjection;
};

[vk::binding(0, 0)]
RWTexture2D<float4> colorImage;
[vk::binding(1, 0)]
RWTexture2D<uint> depthImage;

[vk::binding(0, 1)]
StructuredBuffer<Vertex> vertexBuffer;
[vk::binding(1, 1)]
StructuredBuffer<uint> indexBuffer;

[vk::binding(0, 2)]
ConstantBuffer<Camera> camera;

void ApplyTransformations(inout Triangle triangle) {
    float4 pos1 = mul(camera.projection, mul(camera.view, float4(triangle.p1.position, 1.0)));
    float4 pos2 = mul(camera.projection, mul(camera.view, float4(triangle.p2.position, 1.0)));
    float4 pos3 = mul(camera.projection, mul(camera.view, float4(triangle.p3.position, 1.0)));
    triangle.p1.position = pos1.xyz / pos1.w;
    triangle.p2.position = pos2.xyz / pos2.w;
    triangle.p3.position = pos3.xyz / pos3.w;
}

bool IntersectTriangle(float2 ndcCoords, Triangle triangle, out float3 barycentricCoords, out float depth) {
    barycentricCoords = float3(0.0);

    float2 p1_xy = triangle.p1.position.xy;
    float2 p2_xy = triangle.p2.position.xy;
    float2 p3_xy = triangle.p3.position.xy;
    float2 v0 = p2_xy - p1_xy;
    float2 v1 = p3_xy - p1_xy;
    float2 v2 = ndcCoords - p1_xy;

    float d00 = dot(v0, v0);
    float d01 = dot(v0, v1);
    float d11 = dot(v1, v1);
    float d20 = dot(v2, v0);
    float d21 = dot(v2, v1);
    float denom = d00 * d11 - d01 * d01;


    if (abs(denom) < 1e-6) {
        depth = 1.0;
        return false;
    }

    barycentricCoords.y = (d11 * d20 - d01 * d21) / denom;
    barycentricCoords.z = (d00 * d21 - d01 * d20) / denom;
    barycentricCoords.x = 1.0 - barycentricCoords.y - barycentricCoords.z;

    if (barycentricCoords.x < 0.0 || barycentricCoords.y < 0.0 || barycentricCoords.z < 0.0) {
        depth = 1.0;
        return false;
    }

    depth = barycentricCoords.x * triangle.p1.position.z +
            barycentricCoords.y * triangle.p2.position.z +
            barycentricCoords.z * triangle.p3.position.z;

    if (depth < 0.0 || depth > 1.0) {
        return false;
    }
    return true;
}

[numthreads(16, 16, 1)]
[shader("compute")]
void RasterizeTriangles(uint3 dispatchThreadID : SV_DispatchThreadID) {
    uint2 pixelCoords = dispatchThreadID.xy;
    uint width, height;
    colorImage.GetDimensions(width, height);
    float2 ndcCoords = (float2(pixelCoords) / float2(width, height)) * 2.0 - 1.0;

    if (pixelCoords.x >= width || pixelCoords.y >= height) {
        return;
    }

    uint triangleIndex = dispatchThreadID.z * 3;
    Triangle triangle = {
        vertexBuffer[indexBuffer[triangleIndex]],
        vertexBuffer[indexBuffer[triangleIndex + 1]],
        vertexBuffer[indexBuffer[triangleIndex + 2]]
    };

    ApplyTransformations(triangle);


    float3 barycentricCoords;
    float depth;
    if (!IntersectTriangle(ndcCoords, triangle, barycentricCoords, depth)) {
        return;
    }

    uint depthAsUint = asuint(depth);
    InterlockedMin(depthImage[pixelCoords], depthAsUint);
    if (depthAsUint == depthImage[pixelCoords]) {
        float3 color = barycentricCoords.x * triangle.p1.color +
                       barycentricCoords.y * triangle.p2.color +
                       barycentricCoords.z * triangle.p3.color;
        colorImage[pixelCoords] = float4(color, 1.0);
    }
}