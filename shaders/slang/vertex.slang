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
        return all(position == other.position);
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
groupshared uint local_new_indices[128];

[numthreads(8, 8, 1)]
void sort (
	uint3 GTid : SV_GroupThreadID,
	uint3 DTid : SV_DispatchThreadID,
	uint3 Gid  : SV_GroupID,
	RWStructuredBuffer<Vertex> data,
	RWStructuredBuffer<uint> newIndices,
	uniform Parameters parameters
) {
	uint3 workGroupSize = WorkgroupSize();

	uint t = GTid.x * workGroupSize.y + GTid.y;
	uint t_prime = DTid.x * workGroupSize.y + DTid.y;
	uint offset = Gid.x * 2 * workGroupSize.x * workGroupSize.y;

	if (parameters.algorithm <= Algorithm.LocalDisperse){
		local_value[t * 2]   = data[offset + t * 2];
		local_value[t * 2 + 1] = data[offset + t * 2 + 1];
		local_new_indices[t * 2]   = offset + t * 2;
		local_new_indices[t * 2 + 1] = offset + t * 2 + 1;
	}

	switch (parameters.algorithm){
		case Algorithm.LocalBitonicMergeSort:
			local_bms(parameters.h, t, local_value, local_new_indices);
		break;
		case Algorithm.LocalDisperse:
			local_disperse(parameters.h, t, local_value, local_new_indices);
		break;
		case Algorithm.BigFlip:
			big_flip(parameters.h, t_prime, data, newIndices);
		break;
		case Algorithm.BigDisperse:
			big_disperse(parameters.h, t_prime, data, newIndices);
		break;
	}

	if (parameters.algorithm <= Algorithm.LocalDisperse){
		GroupMemoryBarrierWithGroupSync();
		data[offset + t * 2]   = local_value[t * 2];
		data[offset + t * 2 + 1] = local_value[t * 2 + 1];
		newIndices[offset + t * 2]   = local_new_indices[t * 2];
		newIndices[offset + t * 2 + 1] = local_new_indices[t * 2 + 1];
	}
}