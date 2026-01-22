module perlin;


RWTexture2D<float> perlinTexture;

struct PerlinParams {
    uint octaves;
};

func randomGradient(uint2 uv) -> float2 {
    float angle = frac(sin(dot(float2(uv), float2(12.9898, 78.233))) * 43758.5453) * 2.0 * 3.14159265;
    return float2(cos(angle), sin(angle));
}

func quinticFade(float t) -> float {
    return t * t * t * (t * (t * 6 - 15) + 10);
}

[numthreads(16, 16, 1)]
void generate(
    uint2 pixel : SV_DispatchThreadID,
    uniform PerlinParams params
) {
    uint2 outputSize;
    perlinTexture.GetDimensions(outputSize.x, outputSize.y);

    float2 pixelSize = 1.0 / float2(outputSize - 1);
    float2 uv = float2(pixel) / float2(outputSize) + pixelSize * 0.5;
    perlinTexture[pixel] = 0.0;

    float amplitude = 2.0;
    for (uint octave = 0; octave < params.octaves; octave++) {
        float frequency = pow(2.0, float(octave));
        float2 p = uv * frequency;
        uint2 p0 = uint2(floor(p));
        uint2 p1 = p0 + uint2(1, 0);
        uint2 p2 = p0 + uint2(0, 1);
        uint2 p3 = p0 + uint2(1, 1);
        float2 f = frac(p);
        float2 fade_f = float2(quinticFade(f.x), quinticFade(f.y));
        float2 g0 = randomGradient(p0);
        float2 g1 = randomGradient(p1);
        float2 g2 = randomGradient(p2);
        float2 g3 = randomGradient(p3);
        float d0 = dot(g0, f - float2(0, 0));
        float d1 = dot(g1, f - float2(1, 0));
        float d2 = dot(g2, f - float2(0, 1));
        float d3 = dot(g3, f - float2(1, 1));
        float ix0 = lerp(d0, d1, fade_f.x);
        float ix1 = lerp(d2, d3, fade_f.x);
        float value = lerp(ix0, ix1, fade_f.y);
        perlinTexture[pixel] += value * amplitude;
        amplitude /= 2;
    }
}