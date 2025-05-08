#version 460 core
#extension GL_EXT_buffer_reference : require

layout(location = 0) in vec2 tex_coord;

layout(location = 0) out float frag_color;

#include <ssao/ubo.glsl>

layout(set = 0, binding = 0) uniform S { SSAOUBO ubo; }; // UBO
layout(set = 0, binding = 1) readonly buffer K { vec3 kernels[]; }; // SSBO
layout(set = 0, binding = 2) uniform sampler2D g_position;
layout(set = 0, binding = 3) uniform sampler2D g_normal;
layout(set = 0, binding = 4) uniform sampler2D tex_noise;

void main()
{
	vec2 noise_scale = vec2(ubo.screen_width / ubo.noise_size, ubo.screen_height / ubo.noise_size);

	vec4 frag_pos = texture(g_position, tex_coord).xyzw;
	vec3 normal = normalize(texture(g_normal, tex_coord).rgb);
	vec3 random_vec = normalize(vec3(texture(tex_noise, tex_coord * noise_scale).xy, 0.0));

	vec3 tangent = normalize(random_vec - normal * dot(random_vec, normal));
	vec3 bi_tangent = cross(normal, tangent);
	mat3 tbn = mat3(tangent, bi_tangent, normal);

	float occlusion = 0.0;

	for (int i = 0; i < kernels.length(); ++i)
	{
		// Get sample position
		vec3 sample_pos = tbn * kernels[i]; // From tangent to view-space
		sample_pos = frag_pos.xyz + sample_pos * ubo.radius;

		// Project sample position (to sample texture) (to get position on screen/texture)
		vec4 offset = vec4(sample_pos, 1.0);
		offset = ubo.projection * offset; // From view to clip-space
		offset.xyz /= offset.w; // Perspective divide
		offset.xyz = offset.xyz * 0.5 + 0.5; // Transform to range 0.0 - 1.0

		// Get sample depth
		float sample_depth = texture(g_position, offset.xy).z; // Get depth value of kernel sample

		// w = 1.0 if background, 0.0 if foreground
		float discard_factor = 1.0 - frag_pos.w;

		// Range check & accumulate
		float range_check = smoothstep(0.0, 1.0, ubo.radius / abs(frag_pos.z - sample_depth)) * discard_factor;
		occlusion += (sample_depth >= sample_pos.z + ubo.bias ? 1.0 : 0.0) * range_check;
	}

	occlusion = 1.0 - (occlusion / kernels.length());

	frag_color = pow(occlusion, ubo.power);
}
