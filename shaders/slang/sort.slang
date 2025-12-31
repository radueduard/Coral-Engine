module sort;

#define N_ELEMENTS 16

groupshared float local_value[N_ELEMENTS];

[numthreads(N_ELEMENTS / 2, 1, 1)]
void bitonicSort(
    // uint3 DTid : SV_DispatchThreadID,
    uint3 GTid : SV_GroupThreadID,
    // uint3 GId : SV_GroupID,
    RWStructuredBuffer<float> data,
    RWStructuredBuffer<float> output
) {
    uint t = GTid.x;

	local_value[t * 2]     = data[t * 2];
	local_value[t * 2 + 1] = data[t * 2 + 1];

	GroupMemoryBarrierWithGroupSync();
    for (uint h = 2; h <= N_ELEMENTS; h *= 2) {
        do_flip(h, t);
        for (uint hh = h / 2; hh > 1; hh /= 2) {
            GroupMemoryBarrierWithGroupSync();
            do_disperse(hh, t);
        }
        GroupMemoryBarrierWithGroupSync();
    }

	output[t * 2]     = local_value[t * 2];
	output[t * 2 + 1] = local_value[t * 2 + 1];
}

void local_compare_and_swap(uint2 idx) {
	if (local_value[idx.x] > local_value[idx.y]) {
		float tmp = local_value[idx.x];
		local_value[idx.x] = local_value[idx.y];
		local_value[idx.y] = tmp;
	}
}

void do_flip(uint h, uint t) {
	uint q = ((2 * t) / h) * h;
	uint2 indices = q + uint2(t % h, h - (t % h));
	local_compare_and_swap(indices);
}

void do_disperse(uint h, uint t) {
	uint q = ((2 * t) / h) * h;
	uint2 indices = q + (t % h) + uint2(0, (h / 2));
	local_compare_and_swap(indices);
}