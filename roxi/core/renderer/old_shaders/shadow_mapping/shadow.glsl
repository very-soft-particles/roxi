#include <shadow_mapping/poisson.glsl>

const float SHADOW_AMBIENT = 0.15;
const int BLUR_RANGE = 1;
const float PCF_SCALE = 0.5;
const float POISSON_RADIUS = 5000.0; // Smaller means blurrier

// learnopengl.com/Advanced-Lighting/Shadows/Shadow-Mapping
float shadow_pcf(vec4 shadow_coord)
{
	vec3 n = normalize(normal);
	vec3 l = normalize(shadow_ubo.light_position.xyz - world_pos);
	float nol = dot(n, l);

	float bias = max(shadow_ubo.shadow_max_bias * (1.0 - nol), shadow_ubo.shadow_min_bias);

	// PCF
	ivec2 tex_dim = textureSize(shadow_map, 0).xy;
	float dx = PCF_SCALE / float(tex_dim.x);
	float dy = PCF_SCALE / float(tex_dim.y);
	
	float shadow = 0.0;
	int count = 0;
	for (int x = -BLUR_RANGE; x <= BLUR_RANGE; x++)
	{
		for (int y = -BLUR_RANGE; y <= BLUR_RANGE; y++)
		{
			vec2 off = vec2(dx * x, dy * y);
			float dist = texture(shadow_map, shadow_coord.st + off).r;
			shadow += (shadow_coord.z - bias) > dist ? SHADOW_AMBIENT : 1.0;
			count++;
		}
	}
	return shadow / count;
}

// This is a combination of
// github.com/opengl-tutorials/ogl/blob/master/tutorial16_shadowmaps/ShadowMapping.fragmentshader
// and
// learnopengl.com/Advanced-Lighting/Shadows/Shadow-Mapping
float shadow_poisson(vec4 shadow_coord)
{
	vec3 n = normalize(normal);
	vec3 l = normalize(shadow_ubo.light_position.xyz - world_pos);
	float nol = dot(n, l);

	float bias = max(shadow_ubo.shadow_max_bias * (1.0 - NoL), shadow_ubo.shadow_min_bias);

	ivec2 tex_dim = textureSize(shadow_map, 0).xy;
	float dx = 1.0 / float(tex_dim.x);
	float dy = 1.0 / float(tex_dim.y);

	float shadow = 0.0;
	int count = 0;
	for (int x = -BLUR_RANGE; x <= BLUR_RANGE; x++)
	{
		for (int y = -BLUR_RANGE; y <= BLUR_RANGE; y++)
		{
			vec2 off = vec2(dx * x, dy * y);
			vec2 coord = get_poisson_disk_coord(shadow_coord.st + off, count, POISSON_RADIUS);
			float dist = texture(shadow_map, coord).r;
			shadow += (shadow_coord.z - bias) > dist ? SHADOW_AMBIENT : 1.0;
			count++;
		}
	}
	return shadow / float(count);
}
