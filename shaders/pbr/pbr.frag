#version 460 core

layout(location = 0) in PerVertex {
    vec3 position;
    vec3 normal;
    vec3 tangent;
    vec3 bitangent;
    vec2 texCoord1;
    vec2 texCoord2;
    vec4 color;
} v_in;

layout(location = 0) out vec4 FragColor;

layout(set = 0, binding = 0) uniform Camera {
    mat4 view;
    mat4 projection;
    mat4 inverseView;
    mat4 inverseProjection;
} camera;

layout(set = 0, binding = 1) uniform LightCounts {
    uvec3 counts; // x = point lights, y = directional lights, z = spot lights
} lightCounts;

struct PointLight {
    vec3 position;
    vec3 color;
    vec3 attenuation; // x = constant, y = linear, z = quadratic
    float radius;
};

layout(set = 0, binding = 2) uniform PointLights {
    PointLight data[32];
} pointLights;

struct DirectionalLight {
    vec3 direction;
    vec3 color;
    float intensity;
};

layout(set = 0, binding = 3) uniform DirectionalLights {
    DirectionalLight data[32];
} directionalLights;

struct SpotLight {
    vec3 position;
    vec3 direction;
    vec3 color;
    float innerAngle; // in radians
    float outerAngle; // in radians
    float intensity;
    float radius;
};

layout(set = 0, binding = 4) uniform SpotLights {
    SpotLight data[32];
} spotLights;


layout(set = 1, binding = 0) uniform Material {
    float alphaCutoff;
    uint doubleSided;
    float roughnessFactor;
    float metallicFactor;
    vec3 emissiveFactor;
    vec4 albedo;
} material;

layout(set = 1, binding = 1) uniform sampler2D albedoTexture;
layout(set = 1, binding = 2) uniform sampler2D normalTexture;
layout(set = 1, binding = 3) uniform sampler2D metallicTexture;
layout(set = 1, binding = 4) uniform sampler2D roughnessTexture;
layout(set = 1, binding = 5) uniform sampler2D emissiveTexture;
layout(set = 1, binding = 6) uniform sampler2D ambientOcclusionTexture;

const float M_PI = 3.14159265359;
const float c_MinRoughness = 0.04;

struct PBRInfo
{
    float NdotL;                  // cos angle between normal and light direction
    float NdotV;                  // cos angle between normal and view direction
    float NdotH;                  // cos angle between normal and half vector
    float LdotH;                  // cos angle between light direction and half vector
    float VdotH;                  // cos angle between view direction and half vector
    float perceptualRoughness;    // roughness value, as authored by the model creator (input to shader)
    float metalness;              // metallic value at the surface
    vec3 reflectance0;            // full reflectance color (normal incidence angle)
    vec3 reflectance90;           // reflectance color at grazing angle
    float alphaRoughness;         // roughness mapped to a more linear change in the roughness (proposed by [2])
    vec3 diffuseColor;            // color contribution from diffuse lighting
    vec3 specularColor;           // color contribution from specular lighting
};

vec3 diffuse(PBRInfo pbrInputs)
{
    return pbrInputs.diffuseColor / M_PI;
}


vec3 specularReflection(PBRInfo pbrInputs)
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

vec3 getNormalFromMap() {
    vec2 usedTexCoord = v_in.texCoord1;
    vec3 tangentNormal = texture(normalTexture, usedTexCoord).xyz * 2.0 - 1.0;

    vec3 N = normalize(v_in.normal);
    vec3 T = normalize(v_in.tangent);
    vec3 B = normalize(v_in.bitangent);
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}

vec4 SRGBtoLINEAR(vec4 srgbIn)
{
    #define  MANUAL_SRGB 1
    #define SRGB_FAST_APPROXIMATION 1
	#ifdef MANUAL_SRGB
	#ifdef SRGB_FAST_APPROXIMATION
	vec3 linOut = pow(srgbIn.xyz,vec3(2.2));
    #else //SRGB_FAST_APPROXIMATION
	vec3 bLess = step(vec3(0.04045),srgbIn.xyz);
    vec3 linOut = mix( srgbIn.xyz/vec3(12.92), pow((srgbIn.xyz+vec3(0.055))/vec3(1.055),vec3(2.4)), bLess );
    #endif //SRGB_FAST_APPROXIMATION
	return vec4(linOut,srgbIn.w);;
    #else //MANUAL_SRGB
	return srgbIn;
    #endif //MANUAL_SRGB
}

vec3 DirectionalLightContribution(DirectionalLight light, vec3 n, vec3 v, vec3 inWorldPos, PBRInfo pbrInputs) {
    vec3 l = normalize(-light.direction);
    vec3 h = normalize(l + v);

    float NdotL = clamp(dot(n, l), 0.001, 1.0);
    float NdotV = clamp(abs(dot(n, v)), 0.001, 1.0);
    float NdotH = clamp(dot(n, h), 0.0, 1.0);
    float LdotH = clamp(dot(l, h), 0.0, 1.0);
    float VdotH = clamp(dot(v, h), 0.0, 1.0);

    pbrInputs.NdotL = NdotL;
    pbrInputs.NdotV = NdotV;
    pbrInputs.NdotH = NdotH;
    pbrInputs.LdotH = LdotH;
    pbrInputs.VdotH = VdotH;

    const vec3 u_LightColor = light.intensity * light.color;

    vec3 F = specularReflection(pbrInputs);
    float G = geometricOcclusion(pbrInputs);
    float D = microfacetDistribution(pbrInputs);

    vec3 diffuseContrib = (1.0 - F) * diffuse(pbrInputs);
    vec3 specContrib = F * G * D / (4.0 * NdotL * NdotV);
    return NdotL * u_LightColor * (diffuseContrib + specContrib);
}

vec3 PointLightContribution(PointLight light, vec3 n, vec3 v, vec3 inWorldPos, PBRInfo pbrInputs) {
    vec3 l = normalize(light.position - inWorldPos);
    vec3 h = normalize(l + v);

    float NdotL = clamp(dot(n, l), 0.001, 1.0);
    float NdotV = clamp(abs(dot(n, v)), 0.001, 1.0);
    float NdotH = clamp(dot(n, h), 0.0, 1.0);
    float LdotH = clamp(dot(l, h), 0.0, 1.0);
    float VdotH = clamp(dot(v, h), 0.0, 1.0);

    pbrInputs.NdotL = NdotL;
    pbrInputs.NdotV = NdotV;
    pbrInputs.NdotH = NdotH;
    pbrInputs.LdotH = LdotH;
    pbrInputs.VdotH = VdotH;


    float distanceSq = length(light.position - inWorldPos);
    float radiusSq = light.radius * light.radius;
    float attenuationFactor = 1.0f / (light.attenuation.x + light.attenuation.y * distanceSq + light.attenuation.z * distanceSq * distanceSq) * (1 - distanceSq / radiusSq);
    // account for the radius of the light
    if (distanceSq > light.radius * light.radius) {
        return vec3(0.0); // light is outside the radius, no contribution
    }

    const vec3 u_LightColor = light.color * attenuationFactor;

    vec3 F = specularReflection(pbrInputs);
    float G = geometricOcclusion(pbrInputs);
    float D = microfacetDistribution(pbrInputs);

    vec3 diffuseContrib = (1.0 - F) * diffuse(pbrInputs);
    vec3 specContrib = F * G * D / (4.0 * NdotL * NdotV);

    return clamp(NdotL * u_LightColor * (diffuseContrib + specContrib), 0.0, 1.0);
}

void main() {
    vec4 baseColor = SRGBtoLINEAR(texture(albedoTexture, v_in.texCoord1)) * material.albedo;
    if (baseColor.a < material.alphaCutoff) {
        discard;
    }
    baseColor *= v_in.color;

    float perceptualRoughness = material.roughnessFactor;
    float metallic = material.metallicFactor;

    metallic = texture(metallicTexture, v_in.texCoord1).b * metallic;
    perceptualRoughness = texture(roughnessTexture, v_in.texCoord1).g * perceptualRoughness;

    vec3 f0 = vec3(0.04);
    vec3 diffuseColor = baseColor.rgb * (vec3(1.0) - f0);
    diffuseColor *= (1.0 - metallic);

    float alphaRoughness = perceptualRoughness * perceptualRoughness;
    vec3 specularColor = mix(f0, baseColor.rgb, metallic);

    float reflectance = max(max(specularColor.r, specularColor.g), specularColor.b);
    float reflectance90 = clamp(reflectance * 25.0, 0.0, 1.0);
    vec3 specularEnvironmentR0 = specularColor.rgb;
    vec3 specularEnvironmentR90 = vec3(reflectance90);

    PBRInfo pbrInputs = PBRInfo(
        0.0, // NdotL
        0.0, // NdotV
        0.0, // NdotH
        0.0, // LdotH
        0.0, // VdotH
        perceptualRoughness,
        metallic,
        specularEnvironmentR0,
        specularEnvironmentR90,
        alphaRoughness,
        diffuseColor,
        specularColor
    );

    vec3 cameraPos = (camera.inverseView * vec4(0.0, 0.0, 0.0, 1.0)).xyz;
    vec3 inWorldPos = v_in.position;

    vec3 n = getNormalFromMap();
    n.y *= -1.0f;
    vec3 v = normalize(cameraPos - inWorldPos);

    vec3 color = vec3(0.0);
    for (int i = 0; i < lightCounts.counts.x; ++i) {
        PointLight light = pointLights.data[i];
        color += PointLightContribution(light, n, v, inWorldPos, pbrInputs);
    }

    for (int i = 0; i < lightCounts.counts.y; ++i) {
        DirectionalLight light = directionalLights.data[i];
        color += DirectionalLightContribution(light, n, v, inWorldPos, pbrInputs);
    }


    const float u_OcclusionStrength = 1.0f;
    float ao = texture(ambientOcclusionTexture, v_in.texCoord1).r;
    color = mix(color, color * ao, u_OcclusionStrength);

    vec3 emissive = material.emissiveFactor.rgb;
    emissive *= SRGBtoLINEAR(texture(emissiveTexture, v_in.texCoord1)).rgb;
    color += emissive;

    FragColor = vec4(color, baseColor.a);
}
