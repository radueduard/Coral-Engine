module planet;

import marchingCubes;
import transform;

Sampler3D<float> densityField;
 
struct Camera {
    float4x4 view;
    float4x4 projection;
    float4x4 inverseView;
    float4x4 inverseProjection;
};

ConstantBuffer<Camera> camera;

struct Vertex {
    float4 position : SV_Position;
    float3 normal;
};

groupshared uint vertexCount;
groupshared uint primitiveCount;

#define WORKGROUP_SIZE_X 2
#define WORKGROUP_SIZE_Y 2
#define WORKGROUP_SIZE_Z 2

[numthreads(WORKGROUP_SIZE_X, WORKGROUP_SIZE_Y, WORKGROUP_SIZE_Z)]
[outputtopology("triangle")]
[shader("mesh")]
void generateChunkMesh(
    uint3 groupThreadID : SV_GroupThreadID,
    uint3 groupID : SV_GroupID,
    uint3 dispatchThreadID : SV_DispatchThreadID,
    OutputVertices<Vertex, 256> vertices,
    OutputIndices<uint3, 256> primitives, 
) {
    vertexCount = 0;
    primitiveCount = 0;
    
    const float threshold = 0.9;

    uint3 threadCount = uint3(WORKGROUP_SIZE_X, WORKGROUP_SIZE_Y, WORKGROUP_SIZE_Z);
    uint3 groupCount = WorkgroupCount();
    uint3 totalCount = threadCount * groupCount;
    float3 step = 1.0 / float3(totalCount + 1);
    float3 origin = -float3(totalCount / 2) * step;
 
    GridCell cell;
    uint mask = 0;
    for (int i = 0; i < 8; i++) {
        uint3 offset = uint3(i % 2, (i / 2) % 2, (i / 4) % 2);
        float3 samplePos = float3(dispatchThreadID + offset) / float3(totalCount + 1);
        cell.positions[i] = (float3(dispatchThreadID + offset) - float3(totalCount / 2)) / float3(totalCount + 1);
        cell.densities[i] = densityField.SampleLevel(samplePos, 5);
        if (cell.densities[i] > threshold) {
            mask |= (1 << i);
        }
    }

    Array<Optional<Triangle>, 4> triangles = { none, none, none, none };
    GetTrianglesFromCube(mask, cell, threshold, triangles);

    let mvp = mul(camera.projection, camera.view);

    for (int i = 0; i < triangles.getCount(); i++) {
        if (!triangles[i].hasValue) {
            break;
        }
        Triangle tri = triangles[i].value;
        float3 normal = normalize(cross(tri.p2 - tri.p1, tri.p3 - tri.p1));

        int vertexOffset;
        int primitiveOffset;
        InterlockedAdd(vertexCount, 3, vertexOffset);
        InterlockedAdd(primitiveCount, 1, primitiveOffset);
        vertices[vertexOffset + 0] = Vertex(mul(mvp, float4(tri.p1, 1.0)), normal);
        vertices[vertexOffset + 1] = Vertex(mul(mvp, float4(tri.p2, 1.0)), normal);
        vertices[vertexOffset + 2] = Vertex(mul(mvp, float4(tri.p3, 1.0)), normal);
        primitives[primitiveOffset] = uint3(vertexOffset + 0, vertexOffset + 1, vertexOffset + 2);
    }

    SetMeshOutputCounts(vertexCount, primitiveCount);
}

[shader("pixel")]
float4 planetPixelShader(Vertex input) : SV_Target {
    float3 lightDir = normalize(float3(1, 1, -1));
    float lighting = dot(input.normal, lightDir) * 0.5 + 0.5;
    return float4(lighting, lighting, lighting, 1.0);
}
