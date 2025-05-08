#version 460 core

/*
Fragment shader to filter a cubemap into a diffuse (irradiance) cubemap

This shader is based on 
[1] https://learnopengl.com/
[2] https://github.com/SaschaWillems/Vulkan-glTF-PBR
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

#ifndef PI
#define PI 3.1415926535897932384626433832795
#endif

// These push constants are only used for specular map
layout(push_constant) uniform PushConstantCubeFilter
{
	float roughness;
	uint sample_count;
}
pc_params;

vec3 diffuse(vec3 n)
{
	const float TWO_PI = PI * 2.0;
	const float HALF_PI = PI * 0.5;

	vec3 diffuse_color = vec3(0.0);

	// Tangent space calculation from origin point
	vec3 up = vec3(0.0, 1.0, 0.0);
	vec3 right = normalize(cross(up, n));
	up = normalize(cross(n, right));

	float sample_delta = 0.025;
	uint sample_count = 0u;
	for (float phi = 0.0; phi < TWO_PI; phi += sample_delta)
	{
		for (float theta = 0.0; theta < HALF_PI; theta += sample_delta)
		{
			// Spherical to cartesian (in tangent space)
			vec3 tangent_sample = vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
			// Tangent space to world
			vec3 sample_vec = tangent_sample.x * right + tangent_sample.y * up + tangent_sample.z * n;

			diffuse_color += texture(cube_map, sample_vec).rgb * cos(theta) * sin(theta);
			sample_count++;
		}
	}
	diffuse_color = PI * diffuse_color * (1.0 / float(sample_count));

	return diffuse_color;
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
		write_face(face, diffuse(direction));
	}
}
