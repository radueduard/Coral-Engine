module wireframe;

struct Camera {
    float4x4 view;
    float4x4 projection;
    float4x4 inverseView;
    float4x4 inverseProjection;
}

struct VertexInput
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 texCoord : TEXCOORD0;
};

struct VertexOutput
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 texCoord : TEXCOORD0;
};

[shader("vertex")]
VertexOutput vertexMain(VertexInput input)
{
    VertexOutput output;
    output.position = input.position;
    output.normal = input.normal;
    output.texCoord = input.texCoord;
    return output;
}

// ============================================
// Tessellation Control Shader (Hull Shader)
// ============================================
struct TessControlInput
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 texCoord : TEXCOORD0;
};

struct TessControlOutput
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 texCoord : TEXCOORD0;
};

struct TrianglePatchConstants
{
    float edges[3] : SV_TessFactor;
    float inside : SV_InsideTessFactor;
};


struct QuadPatchConstants
{
    float edges[4] : SV_TessFactor;
    float inside[2] : SV_InsideTessFactor;
};


[shader("hull")]
[domain("triangle")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("patchConstantFunc")]
TessControlOutput hullMain(
    InputPatch<TessControlInput, 3> patch,
    uint id : SV_OutputControlPointID)
{
    TessControlOutput output;
    output.position = patch[id].position;
    output.normal = patch[id].normal;
    output.texCoord = patch[id].texCoord;
    return output;
}

TrianglePatchConstants patchConstantFunc(InputPatch<TessControlInput, 3> patch)
{
    TrianglePatchConstants output;
    output.edges[0] = 1.0;
    output.edges[1] = 1.0;
    output.edges[2] = 1.0;
    output.inside = 1.0;
    return output;
}

// ============================================
// Tessellation Evaluation Shader (Domain Shader)
// ============================================
struct TessEvalInput
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 texCoord : TEXCOORD0;
};

struct TessEvalOutput
{
    float4 position : SV_Position;
    float4 worldPosition : POSITION;
    float3 normal : NORMAL;
    float2 texCoord : TEXCOORD0;
};


[[vk::binding(0)]]
ConstantBuffer<Camera> camera;

[shader("domain")]
[domain("tri")]
TessEvalOutput domainMain(
    TrianglePatchConstants patchConst,
    float3 barycentricCoords : SV_DomainLocation,
    const OutputPatch<TessEvalInput, 3> patch,
    uniform float4x4 model
) {
    TessEvalOutput output;
    
    // Interpolate position using barycentric coordinates
    float3 position = patch[0].position * barycentricCoords.x +
                      patch[1].position * barycentricCoords.y +
                      patch[2].position * barycentricCoords.z;

    // Transform to clip space
    output.worldPosition = mul(model, float4(position, 1.0));
    output.position = mul(camera.projection, mul(camera.view, output.worldPosition));

    // Interpolate normal
    float3 normal = normalize(
        patch[0].normal * barycentricCoords.x +
        patch[1].normal * barycentricCoords.y +
        patch[2].normal * barycentricCoords.z
    );

    output.normal = mul((float3x3)model, normal);
    
    // Interpolate texture coordinates
    output.texCoord = patch[0].texCoord * barycentricCoords.x +
                      patch[1].texCoord * barycentricCoords.y +
                      patch[2].texCoord * barycentricCoords.z;

    return output;
}

struct FragmentInput
{
    float4 position : SV_Position;
    float4 worldPosition : POSITION;
    float3 normal : NORMAL;
    float2 texCoord : TEXCOORD0;
};

bool any2(bool3 v)
{
    return (v.x && v.y) || (v.x && v.z) || (v.y && v.z);
}

[[shader("fragment")]]
float4 fragmentMain(FragmentInput input) : SV_Target
{
    return float4(normalize(input.normal) * 0.5 + 0.5, 1.0);
}