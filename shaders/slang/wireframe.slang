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
    float4 tangent : TANGENT;
    float2 uv0 : TEXCOORD0;
    float2 uv1 : TEXCOORD1;
};

struct VertexOutput
{
    float4 position : SV_Position;
    float3 worldPosition : POSITION;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 bitangent : BITANGENT;
    float2 uv0 : TEXCOORD0;
    float2 uv1 : TEXCOORD1;

};

ConstantBuffer<Camera> camera;

[shader("vertex")]
VertexOutput vertexMain(VertexInput input, uniform float4x4 model)
{
    VertexOutput output;
    float4 worldPosition = mul(model, float4(input.position, 1.0));
    output.worldPosition = worldPosition.xyz;
    output.normal = normalize(mul((float3x3)model, input.normal));
    output.tangent = normalize(mul((float3x3)model, input.tangent.xyz));
    output.bitangent = normalize(cross(output.normal, output.tangent) * input.tangent.w);
    output.uv0 = input.uv0;
    output.uv1 = input.uv1;
    output.position = mul(camera.projection, mul(camera.view, worldPosition));
    return output;
}

struct FragmentInput
{
    float4 position : SV_Position;
    float3 worldPosition : POSITION;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 bitangent : BITANGENT;
    float2 uv0 : TEXCOORD0;
    float2 uv1 : TEXCOORD1;
};

struct Material {
    float alphaCutoff;
    uint doubleSided;
    float roughnessFactor;
    float metallicFactor;
    float3 emissiveFactor;
    float4 baseColorFactor;
};

[[vk::binding(0, 1)]] ConstantBuffer<Material> material;
[[vk::binding(1, 1)]] Sampler2D albedoMap;
[[vk::binding(2, 1)]] Sampler2D normalMap;
[[vk::binding(3, 1)]] Sampler2D metallicMap;
[[vk::binding(4, 1)]] Sampler2D roughnessMap;
[[vk::binding(5, 1)]] Sampler2D emissiveMap;
[[vk::binding(6, 1)]] Sampler2D aoMap;

float3 applyNormalMap(float3 normal, float3 tangent, float3 bitangent, float3 normalMapSample)
{
    float3 n = normalMapSample * 2.0 - 1.0;
    float3x3 TBN = float3x3(tangent, bitangent, normal);
    return normalize(mul(TBN, n));
}

[[shader("fragment")]]
float4 fragmentMain(FragmentInput input) : SV_Target
{
    float4 albedo = albedoMap.Sample(input.uv0);
    float4 normal = normalMap.Sample(input.uv0);
    float metallic = metallicMap.Sample(input.uv1).g;
    float roughness = roughnessMap.Sample(input.uv1).b;
    float3 emissive = emissiveMap.Sample(input.uv1).rgb;
    float ao = aoMap.Sample(input.uv1).r;

    float3 lightDir = normalize(float3(0.5, 0.5, 0.5));
    float3 L = lightDir;
    float3 N = applyNormalMap(normalize(input.normal), normalize(input.tangent), normalize(input.bitangent), normal.rgb);
    float3 V = normalize(camera.inverseView[3].xyz - input.worldPosition);
    float3 H = normalize(L + V);

    float diffuse = max(dot(N, L), 0.0) * roughness;
    float specular = 0.0;
    if (diffuse > 0.0) {
        float NdotH = max(dot(N, H), 0.0);
        specular = pow(NdotH, 1.0 / roughness) * metallic;
    }
    float3 color = albedo.rgb * diffuse + specular * ao + emissive * material.emissiveFactor;
    if (albedo.a < material.alphaCutoff || material.doubleSided != 0) {
        discard;
    }
    // return float4(N * 0.5 + 0.5, max(color.r, 1.0));
    return float4(albedo.rgb, max(color.r, 1.0));
}