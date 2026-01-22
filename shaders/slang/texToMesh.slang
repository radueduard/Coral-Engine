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

    uint4 chunkID;
    uint4 chunkCount;
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
    uint3 groupId : SV_GroupID
) {
    vertexCount = 0;
    indexCount = 0;
    GroupMemoryBarrierWithGroupSync();

    uint3 threadsPerGroup = WorkgroupSize();
    uint3 groupsPerChunk = WorkgroupCount();

    uint3 cellCount = threadsPerGroup * groupsPerChunk * payload.chunkCount.xyz;
    uint3 cellId = payload.chunkID.xyz * groupsPerChunk * threadsPerGroup
                 + groupId * threadsPerGroup
                 + threadId;

    GridCell cell;
    for (int i = 0; i < 8; i++) {
        uint3 offset = uint3(i % 2, (i / 2) % 2, (i / 4) % 2);
        float3 samplePos = float3(cellId + offset) / float3(cellCount);

        cell.positions[i] = lerp(payload.gridMin.xyz, payload.gridMax.xyz, samplePos);
        float noise = densityTexture.SampleLevel(samplePos, 0);

        float radius = 15.0;
        cell.densities[i] = radius - length(cell.positions[i]) + noise * 10.0;
    }

    Array<float3, 12> initialVertices;
    Array<uint3, 5> initialPrimitives;
    uint localVertexCount = 0;
    uint localPrimitiveCount = 0;

    initialVertices = { float3(0.0, 0.0, 0.0) };
    initialPrimitives = { uint3(0, 0, 0) };
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


    Array<uint, 12> facesPerVertex = { 0 };
    Array<float3, 12> normals = { float3(0.0, 0.0, 0.0) };
    Array<float3, 12> tangents = { float3(0.0, 0.0, 0.0) };

    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            for (int k = -1; k <= 1; k++) {
                int3 neighborCellId = int3(cellId) + int3(i, j, k);
                if (any(neighborCellId < 0) || any(neighborCellId >= int3(cellCount))) {
                    continue;
                }

                float3 neighborUVW = float3(neighborCellId) / float3(cellCount);
                GridCell neighborCell;
                for (int vi = 0; vi < 8; vi++) {
                    uint3 offset = uint3(vi % 2, (vi / 2) % 2, (vi / 4) % 2);
                    float3 samplePos = float3(neighborCellId + offset) / float3(cellCount);

                    neighborCell.positions[vi] = lerp(payload.gridMin.xyz, payload.gridMax.xyz, samplePos);
                    float noise = densityTexture.SampleLevel(samplePos, 0);

                    float radius = 15.0;
                    neighborCell.densities[vi] = radius - length(neighborCell.positions[vi]) + noise * 10.0;
                }

                Array<float3, 12> neighborVertices;
                Array<uint3, 5> neighborPrimitives;

                uint neighborVertexCount = 0;
                uint neighborPrimitiveCount = 0;

                neighborVertices = { float3(0.0, 0.0, 0.0) };
                neighborPrimitives = { uint3(0, 0, 0) };

                GetTrianglesFromCube (
                    neighborCell,
                    threshold, 
                    neighborVertices, 
                    neighborPrimitives, 
                    neighborVertexCount, 
                    neighborPrimitiveCount
                );

                for (int ni = 0; ni < neighborPrimitiveCount; ni++) {
                    uint3 tri = neighborPrimitives[ni];
                    float3 v0 = neighborVertices[tri.x];
                    float3 v1 = neighborVertices[tri.y];
                    float3 v2 = neighborVertices[tri.z];
                    float3 faceNormal = normalize(cross(v1 - v0, v2 - v0));
                    
                    float3 faceTangent = normalize(cross(faceNormal, float3(0.0, 1.0, 0.0)));
                    if (length(faceTangent) < 0.001) {
                        faceTangent = normalize(cross(faceNormal, float3(1.0, 0.0, 0.0)));
                    }

                    for (int vj = 0; vj < localVertexCount; vj++) {
                        float3 mainPos = initialVertices[vj];
                        
                        if (length(v0 - mainPos) <= 0.001) {
                            normals[vj] += faceNormal;
                            tangents[vj] += faceTangent;
                            facesPerVertex[vj]++;
                        }
                        if (length(v1 - mainPos) <= 0.001) {
                            normals[vj] += faceNormal;
                            tangents[vj] += faceTangent;
                            facesPerVertex[vj]++;
                        }
                        if (length(v2 - mainPos) <= 0.001) {
                            normals[vj] += faceNormal;
                            tangents[vj] += faceTangent;
                            facesPerVertex[vj]++;
                        }
                    }
                }
            }
        }
    }

    for (int i = 0; i < localVertexCount; i++) {
        normals[i] = normals[i] / float(facesPerVertex[i]);
        tangents[i] = tangents[i] / float(facesPerVertex[i]);
    }

    for (uint i = 0; i < localVertexCount; i++) {
        vertexBuffer[vertexOffset + localVertexOffset + i] = Vertex(
            initialVertices[i], 
            normals[i], 
            float4(tangents[i], 1),
            cubicToUV(initialVertices[i]), 
            float2(0.0, 0.0), 
            float4(float3(cellId) / float3(cellCount), 1.0)
        );
    }

    for (int i = 0; i < localPrimitiveCount; i++) {
        uint3 tri = initialPrimitives[i];
        indexBuffer[indexOffset + localIndexOffset + i * 3 + 0] = vertexOffset + localVertexOffset + tri.x;
        indexBuffer[indexOffset + localIndexOffset + i * 3 + 1] = vertexOffset + localVertexOffset + tri.y;
        indexBuffer[indexOffset + localIndexOffset + i * 3 + 2] = vertexOffset + localVertexOffset + tri.z;
    }

    for (uint i = 0; i < localVertexCount; i++) {
        Vertex v = vertexBuffer[vertexOffset + localVertexOffset + i];
        v.normal = normalize(v.normal);
        vertexBuffer[vertexOffset + localVertexOffset + i] = v;
    }
}