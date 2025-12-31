module perlin;

/*
 * It contains multiple mipmap levels of 3D Perlin noise directions.
 * Each direction is represented as a 3D vector stored in RGB channels.
 * The number of mipmap levels is the same as the octaves used in Perlin noise generation.
 * It is a storage image that is read-only in the shader.
 */
// RWTexture3D<float4> directionTexture[4];

/*
 * Output 3D texture to store the generated Perlin noise values.
 */

[vk::binding(1)]
RWTexture3D<float> perlinTexture;

struct PerlinParams {
    uint octaves;
};

[numthreads(8, 8, 8)]
void generate3D(
    uint3 pixel : SV_DispatchThreadID,
    uniform PerlinParams params
) {
    uint3 outputSize;
    perlinTexture.GetDimensions(outputSize.x, outputSize.y, outputSize.z);
    float3 uvw = float3(pixel) / float3(outputSize);

    static const float3[2][2][2] directions = {
        {
            {
                normalize(float3(-1, -1, -1)), normalize(float3(1, -1, -1))
            }, {
                normalize(float3(-1, 1, -1)), normalize(float3(1, 1, -1))
            }
        }, {
            {
                normalize(float3(-1, -1, 1)), normalize(float3(1, -1, 1))
            }, {
                normalize(float3(-1, 1, 1)), normalize(float3(1, 1, 1))
            }
        },
    };

    perlinTexture[pixel] = 0.0;
    float amplitude = 0.5;

    for (int i = 0; i < params.octaves; i++) {
        perlinTexture[pixel] += amplitude;
        amplitude *= 0.5;
    }
}
