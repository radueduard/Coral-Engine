module skybox;
import camera;

layout(set = 0) ConstantBuffer<Camera> camera;

struct VSInput
{
    float3 position : POSITION;
};

struct PSInput
{
    float4 position : SV_POSITION;
    float3 texcoord : TEXCOORD0;
};

[shader("vertex")]
PSInput vertex(VSInput input)
{
    PSInput output;

    float4x4 viewNoTranslation = transpose(camera.view);
    viewNoTranslation[3] = float4(0.0, 0.0, 0.0, 1.0);
    viewNoTranslation = transpose(viewNoTranslation);

    output.position = mul(camera.projection, mul(viewNoTranslation, float4(input.position, 1.0)));
    output.texcoord = input.position;

    return output;
}

layout(set = 1) SamplerCube<float4> skyboxTexture;

[shader("fragment")]
float4 fragment(PSInput input) : SV_TARGET
{
    return skyboxTexture.Sample(input.texcoord) * .5f;
}