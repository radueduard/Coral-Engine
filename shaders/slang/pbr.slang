module pbr;

import camera;
import vertex;
import material;

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
    RWTexture2DArray<float> depthTextures;
};

layout(set = 2) ShadowMappingData shadowMappingData;

#define M_PI 3.14159265359

struct PBRInfo
{
    float NdotL;                  // cos angle between normal and light direction
    float NdotV;                  // cos angle between normal and view direction
    float NdotH;                  // cos angle between normal and half vector
    float LdotH;                  // cos angle between light direction and half vector
    float VdotH;                  // cos angle between view direction and half vector
    float perceptualRoughness;    // roughness value, as authored by the model creator (input to shader)
    float metalness;              // metallic value at the surface
    float3 reflectance0;            // full reflectance color (normal incidence angle)
    float3 reflectance90;           // reflectance color at grazing angle
    float alphaRoughness;         // roughness mapped to a more linear change in the roughness (proposed by [2])
    float3 diffuseColor;            // color contribution from diffuse lighting
    float3 specularColor;           // color contribution from specular lighting
};

float3 diffuse(PBRInfo pbrInputs)
{
    return pbrInputs.diffuseColor / M_PI;
}

float3 specularReflection(PBRInfo pbrInputs)
{
    return pbrInputs.reflectance0 + (pbrInputs.reflectance90 - pbrInputs.reflectance0) * pow(clamp(1.0 - pbrInputs.VdotH, 0.0, 1.0), 5.0);
}

float geometricOcclusion(PBRInfo pbrInputs)
{
    float NdotL = pbrInputs.NdotL;
    float NdotV = pbrInputs.NdotV;
    float r = pbrInputs.alphaRoughness;

    float attenuationL = 2.0 * NdotL / (NdotL + sqrt(r * r + (1.0 - r * r) * (NdotL * NdotL)));
    float attenuationV = 2.0 * NdotV / (NdotV + sqrt(r * r + (1.0 - r * r) * (NdotV * NdotV)));
    return attenuationL * attenuationV;
}

float microfacetDistribution(PBRInfo pbrInputs)
{
    float roughnessSq = pbrInputs.alphaRoughness * pbrInputs.alphaRoughness;
    float f = (pbrInputs.NdotH * roughnessSq - pbrInputs.NdotH) * pbrInputs.NdotH + 1.0;
    return roughnessSq / (M_PI * f * f);
}

float textureProjection(float3 shadowCoord, float2 offset, RWTexture2DArray<float> shadowMap, uint index)
{
    float shadow = 1.0;
    if (-1.0 < shadowCoord.z && shadowCoord.z < 1.0)
    {

        float depth = shadowMap.Load(int3(int2(shadowCoord.xy + offset), index)).r;
        if (0.0 < shadowCoord.z && depth < shadowCoord.z - 0.01)
            shadow = .25;
    }
    return shadow;
}

float filterPCF(Camera lightCamera, RWTexture2DArray<float> shadowMap, float3 worldPosition, uint index)
{
    uint shadowWidth, shadowHeight, shadowElements;
    shadowMappingData.depthTextures.GetDimensions(shadowWidth, shadowHeight, shadowElements);

    float4 lightSpacePos = mul(lightCamera.projection, mul(lightCamera.view, float4(worldPosition, 1.0)));
    lightSpacePos.xyz /= lightSpacePos.w;
    float2 uv = lightSpacePos.xy * 0.5 + 0.5;
    float2 texelSize = 1.0 / float2(shadowWidth, shadowHeight);
    float3 shadowCoord = float3(uv * float2(shadowWidth, shadowHeight), lightSpacePos.z);
    float shadow = 0.0;
    for (int y = -1; y <= 1; y++) {
        for (int x = -1; x <= 1; x++) {
            shadow += textureProjection(shadowCoord, float2(x, y), shadowMap, index);
        }
    }
    shadow /= 9.0;
    return shadow;
}

[[shader("fragment")]]
float4 fragment(FragmentInput input) : SV_Target
{
    float3 worldPosition = input.worldPosition.xyz / input.worldPosition.w;
    float4 albedo = material.albedoTexture.Sample(input.uv0 * 100);

    float3 lightDir = normalize(float3(-1, 0, 0));
    float lightIntensity = 5;

    float4 normal = material.normalTexture.Sample(input.uv0 * 100);
    float3 N = applyNormalMap(normalize(input.normal), normalize(input.tangent), normalize(input.bitangent), normal.rgb);
    // N.y = -N.y;

    float metallic = material.metallicTexture.Sample(input.uv0 * 100).r;
    float roughness = material.roughnessTexture.Sample(input.uv0 * 100).r;
    float3 emissive = material.emissiveTexture.Sample(input.uv0 * 100).rgb;
    float ao = material.occlusionTexture.Sample(input.uv0 * 100).r;

    float3 L = lightDir;
    float3 V = normalize(worldPosition - input.eye);
    float3 H = normalize(L + V);

    float perceptualRoughness = material.parameters.roughnessFactor * roughness;
    float alphaRoughness = perceptualRoughness * perceptualRoughness;

    float3 f0 = float3(0.04, 0.04, 0.04);

    float3 diffuseColor = albedo.rgb * (1.0 - f0);
    diffuseColor *= 1.0 - metallic;

    float3 specularColor = lerp(f0, albedo.rgb, metallic);

    float reflectance = max(max(specularColor.r, specularColor.g), specularColor.b);
    float reflectance90 = clamp(reflectance * 25.0, 0.0, 1.0);
    float3 specularEnvironmentR0 = specularColor.rgb;
    float3 specularEnvironmentR90 = float3(reflectance90);

    float NdotL = clamp(dot(N, L), 0.001, 1.0);
    float NdotV = clamp(dot(N, V), 0.001, 1.0);
    float NdotH = clamp(dot(N, H), 0.0, 1.0);
    float LdotH = clamp(dot(L, H), 0.0, 1.0);
    float VdotH = clamp(dot(V, H), 0.0, 1.0);

    PBRInfo pbrInputs = {
        NdotL,
        NdotV,
        NdotH,
        LdotH,
        VdotH,
        perceptualRoughness,
        metallic,
        specularEnvironmentR0,
        specularEnvironmentR90,
        alphaRoughness,
        diffuseColor,
        specularColor
    };

    float3 F = specularReflection(pbrInputs);
    float G = geometricOcclusion(pbrInputs);
    float D = microfacetDistribution(pbrInputs);

    float3 lightColor = float3(1.0, 1.0, 1.0);

    float3 diffuseContrib = diffuse(pbrInputs) * (1.0 - F);
    float3 specularContrib = (F * G * D) / (4.0 * NdotL * NdotV);

    float3 color = (diffuseContrib + specularContrib) * lightColor * NdotL * lightIntensity;

    color += emissive;

    color = lerp(color, color * ao, 1.0);

    float shadow = 0.0;
    for (uint i = 0; i < 1; i++) {
        shadow += filterPCF(shadowMappingData.lightCameras[i], shadowMappingData.depthTextures, worldPosition, i);
    }
    shadow = clamp(shadow, 0, 1);

    color *= (shadow);

    color += albedo.rgb * 0.1;

    color = clamp(color, 0, 1);

    return float4(color, 1.0);
}