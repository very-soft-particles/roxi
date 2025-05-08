struct LightCell {
	uint offset;
	uint count;
};

struct ClusteredForwardData {
	vec2 screen_size;
	float slice_scaling;
	float slice_bias;
	uint slice_count_x;
	uint slice_count_y;
	uint slice_count_z;
};
