module blur;


struct BlurPayload<int N> {
    vector<float, N> direction;
    int kernelRadius;
};

// [shader("compute")]
// [numthreads(16, 16, 1)]
// func Gaussian2D(
//     RWTexture2D<float4> inputTexture,
//     RWTexture2D<float4> outputTexture,
//     uniform BlurPayload<2> payload,

//     uint2 pixel : SV_DispatchThreadID
// ) {
//     float sigma = float(payload.kernelRadius) / 2.0;
//     float twoSigmaSq = 2.0 * sigma * sigma;
//     float normalizationFactor = 0.0;

//     uint2 textureSize;
//     inputTexture.GetDimensions(textureSize.x, textureSize.y);

//     Array<float, 32> kernel;
//     for (int i = -payload.kernelRadius; i <= payload.kernelRadius; i++) {
//         float x = float(i);
//         float weight = exp(-(x * x) / twoSigmaSq);
//         kernel[i + payload.kernelRadius] = weight;
//         normalizationFactor += weight;
//     }

//     float4 color = float4(0.0, 0.0, 0.0, 0.0);
//     for (int k = -payload.kernelRadius; k <= payload.kernelRadius; k++)
//     {
//         int2 samplePos = int2(pixel) + int2(payload.direction * float2(k));
//         samplePos = clamp(samplePos, int2(0, 0), int2(textureSize - int2(1, 1)));
//         color += inputTexture.Load(samplePos) * kernel[k + payload.kernelRadius];
//     }
//     color /= normalizationFactor;
//     outputTexture[int2(pixel)] = color;
// }

[shader("compute")]
[numthreads(8, 8, 8)]
func Gaussian3D(
    in RWTexture3D<float4> inputTexture,
    in RWTexture3D<float4> outputTexture,
    uniform BlurPayload<3> payload,

    uint3 pixel : SV_DispatchThreadID
) {
    float sigma = float(payload.kernelRadius) / 2.0;
    float twoSigmaSq = 2.0 * sigma * sigma;
    float normalizationFactor = 0.0;

    uint3 textureSize;
    inputTexture.GetDimensions(textureSize.x, textureSize.y, textureSize.z);

    Array<float, 32> kernel;
    for (int i = -payload.kernelRadius; i <= payload.kernelRadius; i++) {
        float x = float(i);
        float weight = exp(-(x * x) / twoSigmaSq);
        kernel[i + payload.kernelRadius] = weight;
        normalizationFactor += weight;
    }

    float4 color = float4(0.0, 0.0, 0.0, 0.0);
    for (int k = -payload.kernelRadius; k <= payload.kernelRadius; k++)
    {
        int3 samplePos = int3(pixel) + int3(payload.direction * float3(k));
        samplePos = clamp(samplePos, int3(0, 0, 0), int3(textureSize - int3(1, 1, 1)));
        color += inputTexture.Load(samplePos) * kernel[k + payload.kernelRadius];
    }
    color /= normalizationFactor;
    outputTexture[int3(pixel)] = color;
}