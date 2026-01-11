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
};

ConstantBuffer<Camera> camera;

[shader("vertex")]
VertexOutput vertex(Vertex input, uniform float4x4 model)
{
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
    output.eye = camera.view[3].xyz;
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
    if (-1.0 < shadowCoord.z && shadowCoord.z < 1.0)
    {

        float depth = shadowMap.Sample(float3(shadowCoord.xy + offset, index));
        if (0.0 < shadowCoord.z && depth < shadowCoord.z - 0.01)
            shadow = .25;
    }
    return shadow;
}

float filterPCF(Camera lightCamera, Sampler2DArray<float> shadowMap, float3 worldPosition, uint index)
{
    uint shadowWidth, shadowHeight, shadowElements;
    shadowMappingData.depthTextures.GetDimensions(shadowWidth, shadowHeight, shadowElements);

    float4 lightSpacePos = mul(lightCamera.projection, mul(lightCamera.view, float4(worldPosition, 1.0)));
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
    float4 albedo = material.albedoTexture.Sample(input.uv0 * 100);

    float4 normal = material.normalTexture.Sample(input.uv0 * 100);
    float3 N = applyNormalMap(normalize(input.normal), normalize(input.tangent), normalize(input.bitangent), normal.rgb);
    N.y = -N.y;

    float metallic = material.metallicTexture.Sample(input.uv0 * 100).r;
    float roughness = material.roughnessTexture.Sample(input.uv0 * 100).r;

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
        color += lights.directional[i].GetDirectIlumination(worldPosition, input.eye, properties);
    }

    for (uint i = 0; i < lightCounts.spot; i++) {
        color += lights.spot[i].GetDirectIlumination(worldPosition, input.eye, properties);
    }

    float3 emissive = material.emissiveTexture.Sample(input.uv0 * 100).rgb;
    color += emissive;

    float ao = material.occlusionTexture.Sample(input.uv0 * 100).r;
    color = lerp(color, color * ao, 1.0);

    float shadow = 0.0;
    for (uint i = 0; i < lightCounts.shadowCaster; i++) {
        shadow += filterPCF(shadowMappingData.lightCameras[i], shadowMappingData.depthTextures, worldPosition, i);
    } 
    
    if (lightCounts.shadowCaster > 0)
        shadow = clamp(shadow / lightCounts.shadowCaster, 0, 1);

    color *= (shadow);

    color += albedo.rgb * 0.1;

    color = clamp(color, 0, 1);

    return float4(color, 1.0);
}