#version 460 core

#include <infinite_grid/params.glsl>
#include <infinite_grid/header.glsl>

layout (location = 0) in vec2 in_uv;
layout (location = 1) in vec2 in_camera_pos;
layout (location = 0) out vec4 frag_color;

void main()
{
	frag_color = grid_color(in_uv, in_camera_pos);
}
