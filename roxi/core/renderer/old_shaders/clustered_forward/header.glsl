// from HelloVulkan
#extension GL_EXT_shader_atomic_float: enable
struct LightCell
{
	uint offset;
	uint count;
};

struct ClusterForwardUBO
{
	mat4 camera_inverse_projection;
	mat4 camera_view;
	vec2 screen_size;
	float slice_scaling;
	float slice_bias;
	float camera_near;
	float camera_far;
	uint slice_count_x;
	uint slice_count_y;
	uint slice_count_z;
};
