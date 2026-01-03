module texToMesh;

import marchingCubes;
import mappings;

public struct Vertex {
    public float3 position;
    public float3 normal;
    public float4 tangent;
    public float2 uv1;
    public float2 uv2;
    public float4 color;
}


groupshared uint vertexCount;
groupshared uint indexCount;

groupshared uint vertexOffset;
groupshared uint indexOffset;

struct Payload {
    float4 gridMin;
    float4 gridMax;
};

[shader("compute")]
[numthreads(8, 8, 8)]
func TextureToMesh3D(
    in Sampler3D<float> densityTexture,
    in RWStructuredBuffer<Vertex> vertexBuffer,
    in RWStructuredBuffer<uint> indexBuffer,
    in RWStructuredBuffer<uint> counts,

    uniform Payload payload,

    uint3 threadId : SV_GroupThreadID,
    uint3 cellId : SV_DispatchThreadID,
) {
    vertexCount = 0;
    indexCount = 0;
    GroupMemoryBarrierWithGroupSync();

    uint3 groupSize = WorkgroupSize();
    uint3 groupCount = WorkgroupCount();
    uint3 threadCount = groupSize * groupCount;

    float3 uvw = float3(cellId) / float3(threadCount - 1); // used to sample density texture
    float3 texStep = 1.0 / float3(threadCount - 1);

    GridCell cell;
    for (int i = 0; i < 8; i++) {
        uint3 offset = uint3(i % 2, (i / 2) % 2, (i / 4) % 2);
        float3 samplePos = uvw + float3(offset) * texStep;
        samplePos = clamp(samplePos, 0.0, 1.0);

        cell.positions[i] = lerp(payload.gridMin.xyz, payload.gridMax.xyz, samplePos);
        float noise = densityTexture.SampleLevel(samplePos, 0);

        float radius = 15.0;
        cell.densities[i] = radius - length(cell.positions[i]) + noise * 15.0;
    }

    Array<float3, 12> initialVertices; 
    Array<uint3, 5> initialPrimitives;
    uint localVertexCount = 0;
    uint localPrimitiveCount = 0;

    float threshold = 0.5f;
    GetTrianglesFromCube (
        cell, 
        threshold, 
        initialVertices, 
        initialPrimitives, 
        localVertexCount, 
        localPrimitiveCount
    );

    uint localIndexCount = localPrimitiveCount * 3;

    uint localVertexOffset = 0;
    uint localIndexOffset = 0;
    InterlockedAdd(vertexCount, localVertexCount, localVertexOffset);
    InterlockedAdd(indexCount, localIndexCount , localIndexOffset);
    GroupMemoryBarrierWithGroupSync();

    if (all(threadId == uint3(0, 0, 0))) {
        InterlockedAdd(counts[0], vertexCount, vertexOffset);
        InterlockedAdd(counts[1], indexCount, indexOffset);
    }
    GroupMemoryBarrierWithGroupSync();

    for (uint i = 0; i < localVertexCount; i++) {
        float2 uv = cubicToUV(initialVertices[i]);
        vertexBuffer[vertexOffset + localVertexOffset + i] = Vertex(
            initialVertices[i], 
            float3(0.0, 0.0, 0.0), 
            float4(0.0, 0.0, 0.0, 0.0), 
            uv, 
            float2(0.0, 0.0), 
            float4(1.0, 1.0, 1.0, 1.0)
        );
    }

    for (int i = 0; i < localPrimitiveCount; i++) {
        uint3 tri = initialPrimitives[i];
        indexBuffer[indexOffset + localIndexOffset + i * 3 + 0] = vertexOffset + localVertexOffset + tri.x;
        indexBuffer[indexOffset + localIndexOffset + i * 3 + 1] = vertexOffset + localVertexOffset + tri.y;
        indexBuffer[indexOffset + localIndexOffset + i * 3 + 2] = vertexOffset + localVertexOffset + tri.z;
    }
}