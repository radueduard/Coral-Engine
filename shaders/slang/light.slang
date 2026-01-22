module light;

public struct MaterialProperties
{
    public float4 albedoColor;
    public float3 normal;
    public float metallic;
    public float roughness;
};

#define M_PI 3.14159265359

struct PBR
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


    float3 diffuse()
    {
        return diffuseColor / M_PI;
    }

    float3 specularReflection()
    {
        return reflectance0 + (reflectance90 - reflectance0) * pow(clamp(1.0 - VdotH, 0.0, 1.0), 5.0);
    }

    float geometricOcclusion()
    {
        float NdotL = NdotL;
        float NdotV = NdotV;
        float r = alphaRoughness;

        float attenuationL = 2.0 * NdotL / (NdotL + sqrt(r * r + (1.0 - r * r) * (NdotL * NdotL)));
        float attenuationV = 2.0 * NdotV / (NdotV + sqrt(r * r + (1.0 - r * r) * (NdotV * NdotV)));
        return attenuationL * attenuationV;
    }

    float microfacetDistribution()
    {
        float roughnessSq = alphaRoughness * alphaRoughness;
        float f = (NdotH * roughnessSq - NdotH) * NdotH + 1.0;
        return roughnessSq / (M_PI * f * f);
    }

    float3 directLighting()
    {
        float3 F = specularReflection();
        float G = geometricOcclusion();
        float D = microfacetDistribution();

        float3 diffuseContrib = diffuse() * (1.0 - F);
        float3 specularContrib = (F * G * D) / (4.0 * NdotL * NdotV);

        return (diffuseContrib + specularContrib) * NdotL;
    }


}

public interface ILight {
    float3 getDirection(float3 worldPosition);
    public float3 GetDirectIlumination(float3 worldPosition, float3 eye, MaterialProperties material);
};

public struct PointLight : ILight {
    public float3 position;
    public float intensity;
    public float4 color;
    public float3 attenuation;
    public float range;

    private float3 getDirection(float3 worldPosition) {
        return normalize(position.xyz - worldPosition);
    }

    public float3 GetDirectIlumination(float3 worldPosition, float3 eye, MaterialProperties material) {
        float distance = length(position.xyz - worldPosition);
        if (distance > range)
            return float3(0.0, 0.0, 0.0);

        float3 L = getDirection(worldPosition);
        float3 V = normalize(worldPosition - eye);
        float3 H = normalize(L + V);
        float3 N = material.normal;

        float3 f0 = float3(0.04, 0.04, 0.04);

        float3 diffuseColor = material.albedoColor.rgb * (1.0 - f0) * (1.0 - material.metallic);

        float3 specularColor = lerp(f0, material.albedoColor.rgb, material.metallic);

        float reflectance = max(max(specularColor.r, specularColor.g), specularColor.b);
        float reflectance90 = clamp(reflectance * 25.0, 0.0, 1.0);
        float3 specularEnvironmentR0 = specularColor.rgb;
        float3 specularEnvironmentR90 = float3(reflectance90);

        PBR pbr = {
            clamp(dot(N, L), 0.001, 1.0),
            clamp(dot(N, V), 0.001, 1.0),
            clamp(dot(N, H), 0.0, 1.0),
            clamp(dot(L, H), 0.0, 1.0),
            clamp(dot(V, H), 0.0, 1.0),
            material.roughness,
            material.metallic,
            specularEnvironmentR0,
            specularEnvironmentR90,
            material.roughness * material.roughness,
            diffuseColor,
            specularColor
        };

        float finalIntensity = 1.0 / (attenuation.x + attenuation.y * distance + attenuation.z * distance * distance) * intensity;

        return pbr.directLighting() * finalIntensity * color.rgb;
        // return float3(1.0, 1.0, 1.0);
    }
};

public struct DirectionalLight : ILight {
    public float3 direction;
    public float4 color;
    public float intensity;
    public uint shadowIndex;

    private float3 getDirection(float3 worldPosition) {
        return -normalize(direction);
    }

    public float3 GetDirectIlumination(float3 worldPosition, float3 eye, MaterialProperties material) {
        float3 L = getDirection(worldPosition);
        float3 V = -normalize(worldPosition - eye);
        float3 H = normalize(L + V);
        float3 N = material.normal;

        float3 f0 = float3(0.04, 0.04, 0.04);

        float3 diffuseColor = material.albedoColor.rgb * (1.0 - f0) * (1.0 - material.metallic);

        float3 specularColor = lerp(f0, material.albedoColor.rgb, material.metallic);

        float reflectance = max(max(specularColor.r, specularColor.g), specularColor.b);
        float reflectance90 = clamp(reflectance * 25.0, 0.0, 1.0);
        float3 specularEnvironmentR0 = specularColor.rgb;
        float3 specularEnvironmentR90 = float3(reflectance90);

        PBR pbr = {
            clamp(dot(N, L), 0.001, 1.0),
            clamp(dot(N, V), 0.001, 1.0),
            clamp(dot(N, H), 0.0, 1.0),
            clamp(dot(L, H), 0.0, 1.0),
            clamp(dot(V, H), 0.0, 1.0),
            material.roughness,
            material.metallic,
            specularEnvironmentR0,
            specularEnvironmentR90,
            material.roughness * material.roughness,
            diffuseColor,
            specularColor
        };

        return pbr.directLighting() * intensity * color.rgb;
    }
};

public struct SpotLight : ILight {
    public float3 position;
    public float range;
    public float3 direction;
    public float intensity;
    public float4 color;
    public float3 attenuation;
    public float innerConeAngle;
    public float outerConeAngle;
    public uint shadowIndex;

    private float3 getDirection(float3 worldPosition) {
        return normalize(position - worldPosition);
    }

    private float getSpotFactor(float3 worldPosition) {
        float3 L = normalize(worldPosition - position);
        float spotCos = dot(L, normalize(direction));
        float innerCos = cos(radians(innerConeAngle) * 0.5);
        float outerCos = cos(radians(outerConeAngle) * 0.5);

        return smoothstep(outerCos, innerCos, spotCos);
    }

    private float getAttenuation(float3 worldPosition) {
        float distance = length(position - worldPosition);
        // if (distance > range)
        //     return 0.0;

        return 1.0 / (attenuation.x + attenuation.y *distance + attenuation.z * distance * distance);
    }

    public float3 GetDirectIlumination(float3 worldPosition, float3 eye, MaterialProperties material) {
        float3 L = getDirection(worldPosition);
        float3 V = -normalize(worldPosition - eye);
        float3 H = normalize(L + V);
        float3 N = material.normal;

        float3 f0 = float3(0.04, 0.04, 0.04);

        float3 diffuseColor = material.albedoColor.rgb * (1.0 - f0) * (1.0 - material.metallic);

        float3 specularColor = lerp(f0, material.albedoColor.rgb, material.metallic);

        float reflectance = max(max(specularColor.r, specularColor.g), specularColor.b);
        float reflectance90 = clamp(reflectance * 25.0, 0.0, 1.0);
        float3 specularEnvironmentR0 = specularColor.rgb;
        float3 specularEnvironmentR90 = float3(reflectance90);

        PBR pbr = {
            clamp(dot(N, L), 0.001, 1.0),
            clamp(dot(N, V), 0.001, 1.0),
            clamp(dot(N, H), 0.0, 1.0),
            clamp(dot(L, H), 0.0, 1.0),
            clamp(dot(V, H), 0.0, 1.0),
            material.roughness,
            material.metallic,
            specularEnvironmentR0,
            specularEnvironmentR90,
            material.roughness * material.roughness,
            diffuseColor,
            specularColor
        };

        float totalIntensity = getAttenuation(worldPosition) * getSpotFactor(worldPosition) * intensity;
        return pbr.directLighting() * totalIntensity * color.rgb;
    }
};

public struct Lights {
    public StructuredBuffer<PointLight> point;
    public StructuredBuffer<DirectionalLight> directional;
    public StructuredBuffer<SpotLight> spot;
};

