#version 460 core

#include <cube.glsl>
#include <camera_ubo.glsl>
#include <aabb/aabb.glsl>

layout(set = 0, binding = 0)  uniform CameraBlock { CameraUBO cam_ubo; };
layout(set = 0, binding = 1) buffer Boxes { AABB boxes[]; };

// Render an AABB (not rotated)
void main()
{
	AABB box = boxes[gl_InstanceIndex];
	vec3 extents = (box.max_point - box.min_point).xyz * 0.5;
	vec3 box_pos = (box.max_point + box.min_point).xyz * 0.5;
	int idx = CUBE_INDICES[gl_VertexIndex];
	vec3 cube_pos = CUBE_POS[idx];
	cube_pos = vec3(cube_pos.x * extents.x, cube_pos.y * extents.y, cube_pos.z * extents.z);
	cube_pos += box_pos;
	gl_Position = cam_ubo.projection * cam_ubo.view * vec4(cube_pos, 1.0);

}
