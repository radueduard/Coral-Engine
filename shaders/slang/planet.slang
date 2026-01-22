module planet;

import marchingCubes;
import transform;
import camera;
import mappings;

struct Vertex {
    float4 position : SV_Position;
    float3 normal;
    float2 uv;
};

struct AABB {
    float3 min;
    float3 max;
};


layout(binding = 0) Sampler3D<float> densityField;
layout(binding = 1) ConstantBuffer<Camera> camera;

struct MeshPayload {
    uint3 dispatchThreadID;
    uint3 groupCount;
};

[numthreads(1, 1, 1)]
[shader("amplification")]
void cullChunks(
    uint3 dispatchThreadID : SV_DispatchThreadID
) {
    const AABB sceneBounds = { float3(-25.0, -25.0, -25.0), float3(25.0, 25.0, 25.0) };
    AABB chunkBounds = {
        sceneBounds.min + float3(dispatchThreadID) * (sceneBounds.max - sceneBounds.min) / float3(WorkgroupCount()),
        sceneBounds.min + float3(dispatchThreadID + 1) * (sceneBounds.max - sceneBounds.min) / float3(WorkgroupCount())
    };

    if (!camera.IsBoxInFrustum(chunkBounds.min, chunkBounds.max)) {
        return;
    }

    uint3 groupCount = WorkgroupCount();
    var payload = MeshPayload(dispatchThreadID, groupCount);
    DispatchMesh(16, 16, 16, payload);
}


groupshared uint vertexCount;
groupshared uint primitiveCount;

[numthreads(3, 3, 3)]
[outputtopology("triangle")]
[shader("mesh")]
void renderChunkMesh(
    uint3 localThreadID : SV_GroupThreadID,
    uint3 localGroupID : SV_GroupID,

    OutputVertices<Vertex, 256> vertices,
    OutputIndices<uint3, 256> primitives, 
    in payload MeshPayload payload
) {

    vertexCount = 0;
    primitiveCount = 0;
    GroupMemoryBarrierWithGroupSync();
    

    const float threshold = 0.5;

    uint3 localGroupCount = WorkgroupCount();
    uint3 localThreadCount = WorkgroupSize();

    uint3 globalGroupCount = payload.groupCount;

    uint3 totalGroupCount = localGroupCount * globalGroupCount;
    uint3 totalThreadCount = totalGroupCount * localThreadCount;

    uint3 localGroupId = localGroupID * localThreadCount + localThreadID;
    uint3 dispatchThreadID = localGroupId + payload.dispatchThreadID * (localGroupCount * localThreadCount);
    
    // Calculate chunk-relative position
    float3 chunkMin = float3(payload.dispatchThreadID) / float3(payload.groupCount);
    float3 chunkMax = float3(payload.dispatchThreadID + 1) / float3(payload.groupCount);
    float3 chunkSize = chunkMax - chunkMin;
    
    float3 localStep = chunkSize / float3(localGroupCount * localThreadCount);
    
    float3 step = 1.0 / float3(totalThreadCount);
    float3 origin = float3(totalThreadCount / 2) * step;
 
    GridCell cell;
    for (int i = 0; i < 8; i++) {
        uint3 offset = uint3(i % 2, (i / 2) % 2, (i / 4) % 2);
        
        // Texture sampling in [0, 1] space
        float3 samplePos = chunkMin + float3(localGroupId + offset) * localStep;
        samplePos = clamp(samplePos, 0.0, 1.0);
        
        // World position calculation
        const AABB sceneBounds = { float3(-25.0, -25.0, -25.0), float3(25.0, 25.0, 25.0) };
        cell.positions[i] = lerp(sceneBounds.min, sceneBounds.max, samplePos);
            
        float noise = densityField.SampleLevel(samplePos, 0);
        float radius = 15.0;
        cell.densities[i] = radius - length(cell.positions[i]) + noise * 15.0;
    }

    Array<float3, 12> initialVertices; 
    Array<uint3, 5> initialPrimitives;
    uint localVertexCount = 0;
    uint localPrimitiveCount = 0;

    let earlyReturn = GetTrianglesFromCube (
        cell, 
        threshold, 
        initialVertices, 
        initialPrimitives, 
        localVertexCount, 
        localPrimitiveCount
    );

    int vertexOffset = 0;
    int primitiveOffset = 0;

    if (!earlyReturn) {
        InterlockedAdd(vertexCount, localVertexCount, vertexOffset);
        InterlockedAdd(primitiveCount, localPrimitiveCount, primitiveOffset);
    }
    
    GroupMemoryBarrierWithGroupSync();

    SetMeshOutputCounts(vertexCount, primitiveCount);

    if (earlyReturn) {
        return;
    }

    let mvp = mul(camera.projection, camera.view);

    Array<Vertex, 12> transformedVertices;
    for (int i = 0; i < localVertexCount; i++) {
        float2 uv = cubicToUV(initialVertices[i]);

        transformedVertices[i] = Vertex(mul(mvp, float4(initialVertices[i], 1.0)), float3(0.0, 0.0, 0.0), uv);
    }


    for (int i = 0; i < localPrimitiveCount; i++) {
        uint3 tri = initialPrimitives[i];
        uint idx0 = tri.x;
        uint idx1 = tri.y;
        uint idx2 = tri.z;

        float3 normal = normalize(cross(
            initialVertices[idx1] - initialVertices[idx0],
            initialVertices[idx2] - initialVertices[idx0]
        ));

        transformedVertices[idx0].normal += normal;
        transformedVertices[idx1].normal += normal;
        transformedVertices[idx2].normal += normal;
    }

    for (int i = 0; i < localVertexCount; i++) {
        transformedVertices[i].normal = normalize(transformedVertices[i].normal);
        vertices[vertexOffset + i] = transformedVertices[i];
    }

    for (int i = 0; i < localPrimitiveCount; i++) {
        primitives[primitiveOffset + i] = uint3(
            vertexOffset + initialPrimitives[i].x,
            vertexOffset + initialPrimitives[i].y,
            vertexOffset + initialPrimitives[i].z
        );
    }
}

import material;

layout(set = 1) Material material;

[shader("pixel")]
float4 planetPixelShader(
    Vertex input
) : SV_Target {
    float3 albedo = material.albedoTexture.Sample(input.uv * 100).xyz;

    float3 lightDir = normalize(float3(1, 1, -1));
    float lighting = dot(input.normal, lightDir) * 0.5 + 0.5;
    return float4(albedo * lighting, 1.0);
}
