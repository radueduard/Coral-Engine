module perlin3D;


RWTexture3D<float> perlinTexture;

struct PerlinParams {
    uint octaves;
};

func randomGradient(uint3 uv) -> float3 {
    float phi = frac(sin(dot(float3(uv), float3(12.9898, 78.233, 45.164))) * 43758.5453) * 2.0 * 3.14159265;
    float cosTheta = frac(sin(dot(float3(uv), float3(93.9898, 67.345, 12.345))) * 24634.6345) * 2.0 - 1.0;
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);
    return float3(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta); 
}

func quinticFade(float t) -> float {
    return t * t * t * (t * (t * 6 - 15) + 10);
}

[numthreads(8, 8, 8)]
void generate(
    uint3 pixel : SV_DispatchThreadID,
    uniform PerlinParams params
) {
    uint3 outputSize;
    perlinTexture.GetDimensions(outputSize.x, outputSize.y, outputSize.z);
   
    float3 pixelSize = 1.0 / float3(outputSize - 1);
    float3 uv = float3(pixel) / float3(outputSize) + pixelSize * 0.5;
    perlinTexture[pixel] = 0.0;
    float amplitude = 2.0;
    for (uint octave = 0; octave < params.octaves; octave++) {
        float frequency = pow(2.0, float(octave));
        float3 p = uv * frequency;
        uint3 p0 = uint3(floor(p));
        uint3 p1 = p0 + uint3(1, 0, 0);
        uint3 p2 = p0 + uint3(0, 1, 0);
        uint3 p3 = p0 + uint3(1, 1, 0);
        uint3 p4 = p0 + uint3(0, 0, 1);
        uint3 p5 = p0 + uint3(1, 0, 1);
        uint3 p6 = p0 + uint3(0, 1, 1);
        uint3 p7 = p0 + uint3(1, 1, 1);
        float3 f = frac(p);
        float3 fade_f = float3(quinticFade(f.x), quinticFade(f.y), quinticFade(f.z));
        float3 g0 = randomGradient(p0);
        float3 g1 = randomGradient(p1);
        float3 g2 = randomGradient(p2);
        float3 g3 = randomGradient(p3);
        float3 g4 = randomGradient(p4);
        float3 g5 = randomGradient(p5);
        float3 g6 = randomGradient(p6);
        float3 g7 = randomGradient(p7);

        float d0 = dot(g0, f - float3(0, 0, 0));
        float d1 = dot(g1, f - float3(1, 0, 0));
        float d2 = dot(g2, f - float3(0, 1, 0));
        float d3 = dot(g3, f - float3(1, 1, 0));
        float d4 = dot(g4, f - float3(0, 0, 1));
        float d5 = dot(g5, f - float3(1, 0, 1));
        float d6 = dot(g6, f - float3(0, 1, 1));
        float d7 = dot(g7, f - float3(1, 1, 1));

        float ix0 = lerp(d0, d1, fade_f.x);
        float ix1 = lerp(d2, d3, fade_f.x);
        float ix2 = lerp(d4, d5, fade_f.x);
        float ix3 = lerp(d6, d7, fade_f.x);
        float iy0 = lerp(ix0, ix1, fade_f.y);
        float iy1 = lerp(ix2, ix3, fade_f.y);
        float value = lerp(iy0, iy1, fade_f.z);
        perlinTexture[pixel] += value * amplitude;
        amplitude /= 2;
    }
}
