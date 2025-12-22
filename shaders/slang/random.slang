module random;

public static const float PI = 3.14159265;

// Simple hash function to generate a pseudo-random float based on an integer seed.
public func Hash(inout seed: uint) -> float {
    seed = (seed ^ 61) ^ (seed >> 16);
    seed = seed + (seed << 3);
    seed = seed ^ (seed >> 4);
    seed = seed * 0x27d4eb2d;
    seed = seed ^ (seed >> 15);
    return frac(float(seed) / float(0xFFFFFFFF));
}

// Generates a random float in the range [min, max) based on an integer seed.
public func RandomFloat(inout seed: uint, min: float, max: float) -> float {
    return min + (max - min) * Hash(seed);
}

// Generates a random integer in the range [min, max) based on an integer seed.
public func RandomInt(inout seed: uint, min: int, max: int) -> int {
    return min + int(floor((max - min) * Hash(seed)));
}

public func RandomDirection(inout seed: uint) -> float3 {
    float z = RandomFloat(seed, -1.0, 1.0);
    float a = RandomFloat(seed, 0.0, 2.0 * PI);
    float r = sqrt(1.0 - z * z);
    float x = r * cos(a);
    float y = r * sin(a);
    return float3(x, y, z);
}