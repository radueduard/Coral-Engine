module sort;

public void global_compare_and_swap<T: IComparable>(uint2 idx, inout RWStructuredBuffer<T> value) {
	if (value[idx.x] > value[idx.y]) {
		T tmp = value[idx.x];
		value[idx.x] = value[idx.y];
		value[idx.y] = tmp;
	}
}

public void big_flip<T : IComparable>(in uint h, in uint t_prime, inout RWStructuredBuffer<T> value) {
	uint hh = h >> 1;

	uint q = ((2 * t_prime) / h) * h;
	uint x = q + (t_prime % hh);
	uint y = q + h - (t_prime % hh) - 1;

	global_compare_and_swap(uint2(x, y), value);
}

public void big_disperse<T : IComparable>(in uint h, in uint t_prime, inout RWStructuredBuffer<T> value) {
	uint hh = h >> 1;

	uint q = ((2 * t_prime) / h) * h;
	uint x = q + (t_prime % (hh));
	uint y = q + (t_prime % (hh)) + hh;
	
	global_compare_and_swap(uint2(x,y), value);
}

public void local_compare_and_swap<T : IComparable, int N>(uint2 idx, inout T[N] local_value) {
	if (local_value[idx.x] > local_value[idx.y]) {
		T tmp = local_value[idx.x];
		local_value[idx.x] = local_value[idx.y];
		local_value[idx.y] = tmp;
	}
}

public void local_flip<T : IComparable, int N>(uint h, uint t, inout T[N] local_value) {
	GroupMemoryBarrierWithGroupSync();

	uint hh = h >> 1;
	uint2 indices = uint2(h * ((2 * t) / h)) + uint2(t % hh, h - 1 - (t % hh));
	
	local_compare_and_swap<T, N>(indices, local_value);
}

public void local_disperse<T : IComparable, int N>(uint h, uint t, inout T[N] local_value) {
	for (; h > 1 ; h /= 2) {
		GroupMemoryBarrierWithGroupSync();

		uint hh = h >> 1;
		uint2 indices = uint2(h * ((2 * t) / h)) + uint2(t % hh, hh + (t % hh));

		local_compare_and_swap(indices, local_value);
	}
}

public void local_bms<T : IComparable, int N>(uint h, in uint t, inout T[N] local_value) {
	for (uint hh = 2; hh <= h; hh <<= 1) {
		local_flip(hh, t, local_value);
		local_disperse(hh / 2 , t, local_value);
	}
}

public void global_compare_and_swap<T: IComparable>(uint2 idx, inout RWStructuredBuffer<T> value, inout RWStructuredBuffer<uint> remapTable) {
	if (value[idx.x] > value[idx.y]) {
		T tmp = value[idx.x];
		value[idx.x] = value[idx.y];
		value[idx.y] = tmp;

		uint tmp_idx = remapTable[idx.x];
		remapTable[idx.x] = remapTable[idx.y];
		remapTable[idx.y] = tmp_idx;
	}
}

public void big_flip<T : IComparable>(in uint h, in uint t_prime, inout RWStructuredBuffer<T> value, inout RWStructuredBuffer<uint> remapTable) {
	uint hh = h >> 1;

	uint q = ((2 * t_prime) / h) * h;
	uint x = q + (t_prime % hh);
	uint y = q + h - (t_prime % hh) - 1;

	global_compare_and_swap(uint2(x, y), value, remapTable);
}

public void big_disperse<T : IComparable>(in uint h, in uint t_prime, inout RWStructuredBuffer<T> value, inout RWStructuredBuffer<uint> remapTable) {
	uint hh = h >> 1;

	uint q = ((2 * t_prime) / h) * h;
	uint x = q + (t_prime % (hh));
	uint y = q + (t_prime % (hh)) + hh;
	
	global_compare_and_swap(uint2(x,y), value, remapTable);
}

public void local_compare_and_swap<T : IComparable, int N>(uint2 idx, inout T[N] local_value, inout uint[N] local_remapTable) {
	if (local_value[idx.x] > local_value[idx.y]) {
		T tmp = local_value[idx.x];
		local_value[idx.x] = local_value[idx.y];
		local_value[idx.y] = tmp;

		uint tmp_idx = local_remapTable[idx.x];
		local_remapTable[idx.x] = local_remapTable[idx.y];
		local_remapTable[idx.y] = tmp_idx;
	}
}

public void local_flip<T : IComparable, int N>(uint h, uint t, inout T[N] local_value, inout uint[N] local_remapTable) {
	GroupMemoryBarrierWithGroupSync();

	uint hh = h >> 1;
	uint2 indices = uint2(h * ((2 * t) / h)) + uint2(t % hh, h - 1 - (t % hh));
	
	local_compare_and_swap<T, N>(indices, local_value, local_remapTable);
}

public void local_disperse<T : IComparable, int N>(uint h, uint t, inout T[N] local_value, inout uint[N] local_remapTable) {
	for (; h > 1 ; h /= 2) {
		GroupMemoryBarrierWithGroupSync();

		uint hh = h >> 1;
		uint2 indices = uint2(h * ((2 * t) / h)) + uint2(t % hh, hh + (t % hh));

		local_compare_and_swap(indices, local_value, local_remapTable);
	}
}

public void local_bms<T : IComparable, int N>(uint h, in uint t, inout T[N] local_value, inout uint[N] local_remapTable) {
	for (uint hh = 2; hh <= h; hh <<= 1) {
		local_flip(hh, t, local_value, local_remapTable);
		local_disperse(hh / 2 , t, local_value, local_remapTable);
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Variant for at most 128 elements ///////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

groupshared uint local_value_small[128];

[numthreads(8, 8, 1)]
void bitonicSortSmall (
	uint3 GTid : SV_GroupThreadID,
	RWStructuredBuffer<uint> data
) {
	uint t = GTid.x * 8 + GTid.y;

	local_value_small[t * 2]     = ~0;
	local_value_small[t * 2 + 1] = ~0;

	if (t * 2 < data.getCount()) {
		local_value_small[t * 2]     = data[t * 2];
	}
	if (t * 2 + 1 < data.getCount()) {
		local_value_small[t * 2 + 1] = data[t * 2 + 1];
	}

	local_bms(128, t, local_value_small);

	if (t * 2 < data.getCount()) {
		data[t * 2]     = local_value_small[t * 2];
	}
	if (t * 2 + 1 < data.getCount()) {
		data[t * 2 + 1] = local_value_small[t * 2 + 1];
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// General variant ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


public enum Algorithm : uint {
	LocalBitonicMergeSort = 0,
	LocalDisperse = 1,
	BigFlip = 2,
	BigDisperse = 3
};

public struct Parameters {
	public uint algorithm;
	public uint h;
};

groupshared uint local_value[128];

[numthreads(8, 8, 1)]
void bitonicSort (
	uint3 GTid : SV_GroupThreadID,
	uint3 DTid : SV_DispatchThreadID,
	uint3 Gid  : SV_GroupID,
	RWStructuredBuffer<uint> data,
	uniform Parameters parameters
) {
	uint3 workGroupSize = WorkgroupSize();

	uint t = GTid.x * workGroupSize.y + GTid.y;
	uint t_prime = DTid.x * workGroupSize.y + DTid.y;
	uint offset = Gid.x * 2 * workGroupSize.x * workGroupSize.y;

	if (parameters.algorithm <= Algorithm.LocalDisperse){
		local_value[t * 2]   = data[offset + t * 2];
		local_value[t * 2 + 1] = data[offset + t * 2 + 1];
	}

	switch (parameters.algorithm){
		case Algorithm.LocalBitonicMergeSort:
			local_bms(parameters.h, t, local_value);
		break;
		case Algorithm.LocalDisperse:
			local_disperse(parameters.h, t, local_value);
		break;
		case Algorithm.BigFlip:
			big_flip(parameters.h, t_prime, data);
		break;
		case Algorithm.BigDisperse:
			big_disperse(parameters.h, t_prime, data);
		break;
	}

	if (parameters.algorithm <= Algorithm.LocalDisperse){
		GroupMemoryBarrierWithGroupSync();
		data[offset + t * 2]   = local_value[t * 2];
		data[offset + t * 2 + 1] = local_value[t * 2 + 1];
	}
}
