export module marchingCubes;

import marchingCubesData;

public struct Triangle {
    public float3 p1;
    public float3 p2;
    public float3 p3;

    public __init(float3 a, float3 b, float3 c) {
        p1 = a;
        p2 = b;
        p3 = c;
    }
};

public struct GridCell
{
	public float3[8] positions;
	public float[8] densities;

	__init() {
		positions = float3[8](0.0, 0.0, 0.0);
		densities = float[8](0.0);
	}
};

public func GetTrianglesFromCube (
	GridCell cell, 
	float threshold, 
	inout Array<float3, 12> vertices, 
	inout Array<uint3, 5> triangles,
	inout uint vertexCount,
	inout uint triangleCount
) -> bool
{
	uint vertexMask = 0;
	for (int i = 0; i < 8; i++) {
		if (cell.densities[i] >= threshold) {
			vertexMask |= (1 << i);
		}
	}

    var indexList = triangulationTable[vertexMask];
	int intersectionsKey = edgeTable[vertexMask];
	
	Array<uint, 12> indexOffsets = { 0 };
	for (int i = 0; i < 12; i++) {
		if (((intersectionsKey >> i) & 1) == 1) {
			int v1 = edgeVertexIndices[i][0], v2 = edgeVertexIndices[i][1];
			float t = (threshold - cell.densities[v1]) / (cell.densities[v2] - cell.densities[v1]);
			vertices[vertexCount] = lerp(cell.positions[v1], cell.positions[v2], t);
			indexOffsets[i] = vertexCount;
			vertexCount++;
		}
	}

	for (int i = 0; indexList[i] != -1; i += 3) {
        if (indexList[i] >= 0 && indexList[i] < 12 &&
            indexList[i+1] >= 0 && indexList[i+1] < 12 &&
            indexList[i+2] >= 0 && indexList[i+2] < 12) {
            triangles[triangleCount++] = uint3(
                indexOffsets[indexList[i]], 
                indexOffsets[indexList[i + 1]], 
                indexOffsets[indexList[i + 2]]
            );
        }
    }
    
	return triangleCount == 0;
}