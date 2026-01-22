module pbr;

import camera;
import vertex;
import material;
import light;

struct VertexOutput
{
    float4 position : SV_Position;
    float4 worldPosition : POSITION;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 bitangent : BITANGENT;
    float2 uv0 : TEXCOORD0;
    float2 uv1 : TEXCOORD1;
    float3 eye : EYE;
    float4 color : COLOR;
};

ConstantBuffer<Camera> camera;

[shader("vertex")]
VertexOutput vertex(Vertex input, uniform float4x4 model)
{/*  */
    float3 bitangent = normalize(cross(input.normal, input.tangent.xyz) * input.tangent.w);

    VertexOutput output;
    float4 worldPosition = mul(model, float4(input.position, 1.0));
    output.worldPosition = worldPosition;
    output.normal = normalize(mul((float3x3)model, input.normal));
    output.tangent = normalize(mul((float3x3)model, input.tangent.xyz));
    output.bitangent = normalize(mul((float3x3)model, bitangent));
    output.uv0 = input.uv0;
    output.uv1 = input.uv1;
    output.position = mul(camera.projection, mul(camera.view, worldPosition));
    output.eye = -transpose(camera.view)[3].xyz;
    output.color = input.color;
    return output;
}

struct FragmentInput
{
    float4 position : SV_Position;
    float4 worldPosition : POSITION;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 bitangent : BITANGENT;
    float2 uv0 : TEXCOORD0;
    float2 uv1 : TEXCOORD1;
    float3 eye : EYE;
    float4 color : COLOR;
};


layout(set = 1) Material material;

float3 applyNormalMap(float3 normal, float3 tangent, float3 bitangent, float3 normalMapSample)
{
    float3 n = normalize(normalMapSample * 2.0 - 1.0);
    float3x3 TBN = float3x3(tangent, bitangent, normal);
    return normalize(mul(TBN, n));
}

struct ShadowMappingData
{
    StructuredBuffer<Camera> lightCameras;
    Sampler2DArray<float> depthTextures;
};

layout(set = 2) ShadowMappingData shadowMappingData;



float textureProjection(float3 shadowCoord, float2 offset, Sampler2DArray<float> shadowMap, uint index)
{
    float shadow = 1.0;
    float2 uv = float2(shadowCoord.x + offset.x, shadowCoord.y + offset.y);
    
    // Check if within valid UV range AND valid depth range
    if (uv.x >= 0.0 && uv.x <= 1.0 && 
        uv.y >= 0.0 && uv.y <= 1.0 && 
        shadowCoord.z > 0.0 && shadowCoord.z < 1.0)
    {
        float3 sampleCoord = float3(uv, float(index));
        float depth = shadowMap.SampleLevel(sampleCoord, 0);
        if (depth < shadowCoord.z - 0.01)
            shadow = 0.25;
    }
    return shadow;
}

float filterPCF(StructuredBuffer<Camera> lightCameras, Sampler2DArray<float> shadowMap, float3 worldPosition, uint index)
{
    uint shadowWidth, shadowHeight, shadowElements;
    shadowMap.GetDimensions(shadowWidth, shadowHeight, shadowElements);

    float4 lightSpacePos = mul(lightCameras[index].projection, mul(lightCameras[index].view, float4(worldPosition, 1.0)));
    lightSpacePos.xyz /= lightSpacePos.w;
    float2 uv = lightSpacePos.xy * 0.5 + 0.5;
    float2 texelSize = 1.0 / float2(shadowWidth, shadowHeight);
    float3 shadowCoord = float3(uv, lightSpacePos.z);
    float shadow = 0.0;
    for (int y = -1; y <= 1; y++) {
        for (int x = -1; x <= 1; x++) {
            shadow += textureProjection(shadowCoord, float2(x, y) * texelSize, shadowMap, index);
        }
    }
    shadow /= 9.0;
    return shadow;
}

layout(set = 3) Lights lights;

struct LightCounts
{  
    [[vk::offset(64)]]
    uint point;
    uint directional;
    uint spot;
    uint shadowCaster;
};

[[shader("fragment")]]
float4 fragment(
    FragmentInput input,
    uniform LightCounts lightCounts,
) : SV_Target
{
    float3 worldPosition = input.worldPosition.xyz / input.worldPosition.w;
    float4 albedo = material.albedoTexture.Sample(input.uv0 * 50) * material.parameters.baseColorFactor;

    float4 normal = material.normalTexture.Sample(input.uv0 * 50);
    float3 N = applyNormalMap(normalize(input.normal), normalize(input.tangent), normalize(input.bitangent), normal.rgb);
    N.y = -N.y;

    float metallic = material.metallicTexture.Sample(input.uv0 * 50).r * material.parameters.metallicFactor;
    float roughness = material.roughnessTexture.Sample(input.uv0 * 50).r * material.parameters.roughnessFactor;

    MaterialProperties properties = {
        albedo,
        N,
        metallic,
        material.parameters.roughnessFactor * roughness
    };


    float3 color = float3(0.0, 0.0, 0.0);

    for (uint i = 0; i < lightCounts.point; i++) {
        color += lights.point[i].GetDirectIlumination(worldPosition, input.eye, properties);
    }

    for (uint i = 0; i < lightCounts.directional; i++) {
        int shadowIndex = lights.directional[i].shadowIndex;
        float shadow = filterPCF(shadowMappingData.lightCameras, shadowMappingData.depthTextures, worldPosition, shadowIndex);
        color += lights.directional[i].GetDirectIlumination(worldPosition, input.eye, properties) * shadow;
    }

    for (uint i = 0; i < lightCounts.spot; i++) {
        int shadowIndex = lights.spot[i].shadowIndex;
        float shadow = filterPCF(shadowMappingData.lightCameras, shadowMappingData.depthTextures, worldPosition, shadowIndex);
        color += lights.spot[i].GetDirectIlumination(worldPosition, input.eye, properties) * shadow;
    }

    float3 emissive = material.emissiveTexture.Sample(input.uv0 * 50).rgb;
    color += emissive;

    float ao = material.occlusionTexture.Sample(input.uv0 * 50).r;
    color = lerp(color, color * ao, 1.0);

    color += albedo.rgb * 0.1;

    color = clamp(color, 0, 1);

    // return float4((input.normal + 1.0) * 0.5
    // return float4(input.color.rgb
    //     , max(color.r, 1.0));
    return float4(color, 1.0);
}