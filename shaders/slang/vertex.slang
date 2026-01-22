module vertex;

import sort;

public struct Vertex : IComparable
{
    public float3 position : POSITION;
    public float3 normal : NORMAL;
    public float4 tangent : TANGENT;
    public float2 uv0 : TEXCOORD0;
    public float2 uv1 : TEXCOORD1;
    public float4 color : COLOR;

    func equals(other : Vertex) -> bool {
        return all(abs(position - other.position) < 0.0001);
    }

    func lessThan(other : Vertex) -> bool {
        if (position.x != other.position.x) {
            return position.x < other.position.x;
        } else if (position.y != other.position.y) {
            return position.y < other.position.y;
        } else {
            return position.z < other.position.z;
        }
    }

    func lessThanOrEquals(other : Vertex) -> bool {
        return lessThan(other) || equals(other);
    }
};

groupshared Vertex local_value[128];
groupshared uint local_remapTable[128];

[numthreads(8, 8, 1)]
[shader("compute")]
void sort (
	uint3 GTid : SV_GroupThreadID,
	uint3 DTid : SV_DispatchThreadID,
	uint3 Gid  : SV_GroupID,
	RWStructuredBuffer<Vertex> data,
	RWStructuredBuffer<uint> remapTable,
	uniform Parameters parameters
) {
	uint3 workGroupSize = WorkgroupSize();

	uint t = GTid.x * workGroupSize.y + GTid.y;
	uint t_prime = DTid.x * workGroupSize.y + DTid.y;
	uint offset = Gid.x * 2 * workGroupSize.x * workGroupSize.y;

	if (parameters.algorithm <= Algorithm.LocalDisperse){
		local_value[t * 2]   = data[offset + t * 2];
		local_value[t * 2 + 1] = data[offset + t * 2 + 1];
		local_remapTable[t * 2]   = remapTable[offset + t * 2];
		local_remapTable[t * 2 + 1] = remapTable[offset + t * 2 + 1];
	}

	switch (parameters.algorithm){
		case Algorithm.LocalBitonicMergeSort:
			local_bms(parameters.h, t, local_value, local_remapTable);
		break;
		case Algorithm.LocalDisperse:
			local_disperse(parameters.h, t, local_value, local_remapTable);
		break;
		case Algorithm.BigFlip:
			big_flip(parameters.h, t_prime, data, remapTable);
		break;
		case Algorithm.BigDisperse:
			big_disperse(parameters.h, t_prime, data, remapTable);
		break;
	}

	if (parameters.algorithm <= Algorithm.LocalDisperse){
		GroupMemoryBarrierWithGroupSync();
		data[offset + t * 2]   = local_value[t * 2];
		data[offset + t * 2 + 1] = local_value[t * 2 + 1];
		remapTable[offset + t * 2]   = local_remapTable[t * 2];
		remapTable[offset + t * 2 + 1] = local_remapTable[t * 2 + 1];
	}
}

[numthreads(128, 1, 1)]
[shader("compute")]
void reverseRemapTable(
	uint3 DTid : SV_DispatchThreadID,
	in RWStructuredBuffer<uint> reversedRemapTable,
	in RWStructuredBuffer<uint> resultRemapTable
) {
	if (DTid.x >= resultRemapTable.getCount()) {
		return;
	}

	uint index = DTid.x;
	uint oldPosition = reversedRemapTable[index];
	resultRemapTable[oldPosition] = index;
}

[numthreads(128, 1, 1)]
[shader("compute")]
void actualizeIndices(
	uint3 DTid : SV_DispatchThreadID,
	in RWStructuredBuffer<uint> indices,
	in RWStructuredBuffer<uint> remapTable
) {
	if (DTid.x >= indices.getCount()) {
		return;
	}

	uint index = DTid.x;
	uint oldPosition = indices[index];
	indices[index] = remapTable[oldPosition];
}

#define MAX_FLOAT 3.402823466e+38F

[numthreads(1, 1, 1)]
[shader("compute")]
void combineDuplicates(
	uint3 DTid : SV_DispatchThreadID,
	in RWStructuredBuffer<Vertex> vertices,
	in RWStructuredBuffer<uint> remapTable
) {
	uint offset = DTid.x * 128;
	uint baseVertexIndex = offset;
	Vertex baseVertex = vertices[offset];

	int i = 0;
	while (true) {
		Vertex currentVertex = vertices[offset + i];
		if (length(baseVertex.position - currentVertex.position) < 0.0001) {
			remapTable[offset + i] = baseVertexIndex;
			baseVertex.normal += currentVertex.normal;
			baseVertex.tangent += currentVertex.tangent;

			if (all(baseVertex.position == float3(MAX_FLOAT))) {
				break;
			}
		} else {
			baseVertex.normal = normalize(baseVertex.normal);
			baseVertex.tangent = normalize(baseVertex.tangent);

			vertices[baseVertexIndex] = baseVertex;
			baseVertex = currentVertex;
			baseVertexIndex = offset + i;

			if (i >= 127) {
				break;
			}
		}
		i++;
	}
	baseVertex.normal = normalize(baseVertex.normal);
	baseVertex.tangent = normalize(baseVertex.tangent);

	vertices[baseVertexIndex] = baseVertex;
}
