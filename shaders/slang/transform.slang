module transform;

public func Translate(float3 position) -> float4x4 {
    return float4x4(
        1, 0, 0, position.x,
        0, 1, 0, position.y,
        0, 0, 1, position.z,
        0, 0, 0, 1
    );
}

public func Scale(float3 scale) -> float4x4 {
    return float4x4(
        scale.x, 0, 0, 0,
        0, scale.y, 0, 0,
        0, 0, scale.z, 0,
        0, 0, 0, 1
    );
}

public func RotateX(float angle) -> float4x4 {
    float c = cos(angle);
    float s = sin(angle);
    return float4x4(
        1, 0, 0, 0,
        0, c, -s, 0,
        0, s, c, 0,
        0, 0, 0, 1
    );
}

public func RotateY(float angle) -> float4x4 {
    float c = cos(angle);
    float s = sin(angle);
    return float4x4(
        c, 0, s, 0,
        0, 1, 0, 0,
        -s, 0, c, 0,
        0, 0, 0, 1
    );
}

public func RotateZ(float angle) -> float4x4 {
    float c = cos(angle);
    float s = sin(angle);
    return float4x4(
        c, -s, 0, 0,
        s, c, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    );
}