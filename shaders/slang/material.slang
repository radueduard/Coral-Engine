module material;

public struct MaterialData {
    float alphaCutoff;
    uint doubleSided;
    float roughnessFactor;
    float metallicFactor;
    float3 emissiveFactor;
    float4 baseColorFactor;
}

public struct Material {
    public ConstantBuffer<MaterialData> materialData;
    public Sampler2D albedoTexture;
    public Sampler2D normalTexture;
    public Sampler2D metallicTexture;
    public Sampler2D roughnessTexture;
    public Sampler2D emissiveTexture;
    public Sampler2D occlusionTexture;
}