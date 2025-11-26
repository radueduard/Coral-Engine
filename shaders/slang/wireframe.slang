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

struct PatchConstants
{
    float edges[3] : SV_TessFactor;
    float inside : SV_InsideTessFactor;
};

[shader("hull")]
[domain("tri")]
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

PatchConstants patchConstantFunc(
    InputPatch<TessControlInput, 3> patch)
{
    PatchConstants output;
    // Tessellation levels (passthrough uses fixed values)
    output.edges[0] = 2.0;
    output.edges[1] = 2.0;
    output.edges[2] = 2.0;
    output.inside = 2.0;
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
    float3 normal : NORMAL;
    float2 texCoord : TEXCOORD0;
};

[[vk::binding(0)]]
ConstantBuffer<Camera> camera;

[shader("domain")]
[domain("tri")]
TessEvalOutput domainMain(
    PatchConstants patchConst,
    float3 baryCoords : SV_DomainLocation,
    const OutputPatch<TessEvalInput, 3> patch,
    uniform float4x4 model)
{
    TessEvalOutput output;
    
    // Interpolate position using barycentric coordinates
    float3 pos = patch[0].position * baryCoords.x +
                 patch[1].position * baryCoords.y +
                 patch[2].position * baryCoords.z;
    
    // Interpolate normal
    float3 norm = patch[0].normal * baryCoords.x +
                  patch[1].normal * baryCoords.y +
                  patch[2].normal * baryCoords.z;
    
    // Interpolate texture coordinates
    float2 tc = patch[0].texCoord * baryCoords.x +
                patch[1].texCoord * baryCoords.y +
                patch[2].texCoord * baryCoords.z;
    
    float4x4 modelViewProj = mul(camera.projection, mul(camera.view, model));
    float3 position = normalize(pos);
    output.position = mul(modelViewProj, float4(position, 1.0));
    output.normal = normalize(mul((float3x3)model, norm));
    output.texCoord = tc;
    
    return output;
}

[[shader("fragment")]]
float4 fragmentMain(TessEvalOutput input) : SV_Target
{
    return float4(1.0, 1.0, 1.0, 1.0);
}