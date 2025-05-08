#version 460 core

#include <camera_ubo.glsl>
#include <infinite_grid/params.glsl>

layout (location = 0) out vec2 out_uv;
layout (location = 1) out vec2 out_camera_pos;

layout(push_constant) uniform PC
{
	// This sets the height of the grid
	float y_position;
};

layout(set = 0, binding = 0) uniform CameraBlock { CameraUBO cam_ubo; };

void main()
{
	mat4 mvp = cam_ubo.projection * cam_ubo.view;
	int idx = VERTEX_INDICES[gl_VertexIndex];
	
	vec3 position = VERTEX_POS[idx] * GRID_EXTENTS;
	position.y = y_position;
	
	out_camera_pos = cam_ubo.position.xy;

	position.x += cam_ubo.position.x;
	position.z += cam_ubo.position.y;

	out_uv = position.xz;
	gl_Position = mvp * vec4(position, 1.0);
}
