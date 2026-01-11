module mappings;

static const float PI = 3.14159265359;

public func sphericalToUV(float3 position) -> float2 {
    float3 normalizedPos = normalize(position);
    float phi = atan2(normalizedPos.z, normalizedPos.x);
    float theta = acos(clamp(normalizedPos.y, -1.0, 1.0));
    
    float u = phi / (2.0 * PI);
    float v = theta / PI;
    return float2(u, v);
}

public func cylindricalToUV(float3 position) -> float2 {
    float3 normalizedPos = normalize(position);
    float phi = atan2(normalizedPos.z, normalizedPos.x);
    float u = (phi + PI) / (2.0 * PI);
    float v = (normalizedPos.y + 1.0) / 2.0;
    return float2(u, v);
}

public func planarToUV(float3 position) -> float2 {
    float u = (position.x + 1.0) / 2.0;
    float v = (position.z + 1.0) / 2.0;
    return float2(u, v);
}

public func cubicToUV(float3 position) -> float2 {
    float3 absPos = abs(position);
    float2 uv;
    if (absPos.x >= absPos.y && absPos.x >= absPos.z) {
        uv = float2(
            (position.z / absPos.x + 1.0) / 2.0,
            (position.y / absPos.x + 1.0) / 2.0
        );
    } else if (absPos.y >= absPos.x && absPos.y >= absPos.z) {
        uv = float2(
            (position.x / absPos.y + 1.0) / 2.0,
            (position.z / absPos.y + 1.0) / 2.0
        );
    } else {
        uv = float2(
            (position.x / absPos.z + 1.0) / 2.0,
            (position.y / absPos.z + 1.0) / 2.0
        );
    }
    return uv;
}