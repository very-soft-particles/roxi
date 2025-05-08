#version 460 core

/*
Fragment shader to turn an equirectangular image to a cubemap 

This shader is based on:
[1] https://www.turais.de/how-to-load-hdri-as-a-cubemap-in-opengl/
[2] https://en.wikipedia.org/wiki/Cube_mapping#Memory_addressing
*/

#ifndef PI
#define PI 3.1415926535897932384626433832795
#endif

layout(location = 0) in vec2 tex_coord;

// Multiple render targets
layout(location = 0) out vec4 cube_face_0;
layout(location = 1) out vec4 cube_face_1;
layout(location = 2) out vec4 cube_face_2;
layout(location = 3) out vec4 cube_face_3;
layout(location = 4) out vec4 cube_face_4;
layout(location = 5) out vec4 cube_face_5;

layout(set = 0, binding = 0) uniform sampler2D env_map;

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

// https://en.wikipedia.org/wiki/Cube_mapping#Memory_addressing
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

// Convert Cartesian direction vector to spherical coordinates.
vec2 dir_to_uv(vec3 dir)
{
	return vec2
	(
		0.5f + 0.5f * atan(dir.z, dir.x) / PI, // phi
		1.f - acos(dir.y) / PI // theta
	);
}

void main()
{
	vec2 tex_coord_temp = tex_coord * 2.0 - 1.0;

	for (int face = 0; face < 6; ++face)
	{
		vec3 position = uv_to_xyz(face, tex_coord_temp);
		vec3 direction = normalize(position);
		direction.y = -direction.y;
		vec2 final_tex_coord = dir_to_uv(direction);
		write_face(face, texture(env_map, final_tex_coord).rgb);
	}
}
