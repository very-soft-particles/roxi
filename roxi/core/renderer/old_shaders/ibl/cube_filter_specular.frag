#version 460 core

/*
Fragment shader to filter a cubemap into a specular (prefilter) cubemap

This shader is based on 
[1] https://learnopengl.com/
[2] http://www.codinglabs.net/article_physically_based_rendering.aspx
*/

layout(location = 0) in vec2 tex_coord;

// Multiple render targets
layout(location = 0) out vec4 cube_face_0;
layout(location = 1) out vec4 cube_face_1;
layout(location = 2) out vec4 cube_face_2;
layout(location = 3) out vec4 cube_face_3;
layout(location = 4) out vec4 cube_face_4;
layout(location = 5) out vec4 cube_face_5;

layout(set = 0, binding = 0) uniform samplerCube cube_map;

#include <ibl/header.glsl>
#include <pbr/hammersley.glsl>
#include <pbr/pbr_header.glsl>

layout(push_constant) uniform PushConstantCubeFilter
{
	float roughness;
	uint sample_count;
}
pc_params;

vec3 specular(vec3 n)
{
	// Make the simplifying assumption that V equals R equals the normal 
	vec3 r = n;
	vec3 v = r;

	vec2 texel_size = 1.0 / textureSize(cube_map, 0);
	float sa_texel = 4.0 * PI / (6.0 * texel_size.x * texel_size.x);

	vec3 specular_color = vec3(0.0);
	float total_weight = 0.0;

	for (uint i = 0u; i < pc_params.sample_count; ++i)
	{
		// Generates a sample vector that's biased towards
		// the preferred alignment direction (importance sampling).
		vec2 xi = hammersley(i, pc_params.sample_count);
		vec3 h = importance_sample_ggx(xi, n, pc_params.roughness);
		vec3 l = normalize(2.0 * dot(v, h) * h - v);

		float nol = max(dot(n, l), 0.0);

		if (nol > 0.0)
		{
			float noh = max(dot(n, h), 0.0);
			float hov = max(dot(h, v), 0.0);

			// Sample from the environment's mip level based on roughness/pdf
			float d = distribution_ggx(noh, pc_params.roughness); // Trowbridge-Reitz GGX
			float pdf = D * noh / (4.0 * hov) + 0.0001;
			float sa_sample = 1.0 / (float(pc_params.sample_count) * pdf + 0.0001);
			float mip_level = pc_params.roughness == 0.0 ? 0.0 : 0.5 * log2(sa_sample / sa_texel);

			specular_color += textureLod(cube_map, L, mip_level).rgb * nol;
			total_weight += nol;
		}
	}

	specular_color = specular_color / total_weight;

	return specular_color;
}

void write_face(int face, vec3 color_in)
{
	vec4 color = vec4(color_in.rgb, 1.0f);

	if (face == 0)
	{
		cube_face_0 = color;
	}
	else if (face == 1)
	{
		cube_face_1 = color;
	}
	else if (face == 2)
	{
		cube_face_2 = color;
	}
	else if (face == 3)
	{
		cube_face_3 = color;
	}
	else if (face == 4)
	{
		cube_face_4 = color;
	}
	else
	{
		cube_face_5 = color;
	}
}

vec3 uv_to_xyz(int face, vec2 uv)
{
	if (face == 0)
	{
		return vec3(1.f, uv.y, -uv.x);
	}
	else if (face == 1)
	{
		return vec3(-1.f, uv.y, uv.x);
	}
	else if (face == 2)
	{
		return vec3(+uv.x, -1.f, +uv.y);
	}
	else if (face == 3)
	{
		return vec3(+uv.x, 1.f, -uv.y);
	}
	else if (face == 4)
	{
		return vec3(+uv.x, uv.y, 1.f);
	}
	else
	{
		return vec3(-uv.x, +uv.y, -1.f);
	}
}

// Entry point
void main()
{
	vec2 tex_coord_new = tex_coord * 2.0 - 1.0;

	for (int face = 0; face < 6; ++face)
	{
		vec3 scan = uv_to_xyz(face, tex_coord_new);
		vec3 direction = normalize(scan);
		direction.y = -direction.y;
		write_face(face, specular(direction));
	}
}
