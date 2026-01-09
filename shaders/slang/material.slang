module material;

public struct MaterialData {
    public float alphaCutoff;
    public uint doubleSided;
    public float roughnessFactor;
    public float metallicFactor;
    public float3 emissiveFactor;
    public float4 baseColorFactor;
}

public struct Material {
    public ConstantBuffer<MaterialData> parameters;
    public Sampler2D albedoTexture;
    public Sampler2D normalTexture;
    public Sampler2D metallicTexture;
    public Sampler2D roughnessTexture;
    public Sampler2D emissiveTexture;
    public Sampler2D occlusionTexture;
}