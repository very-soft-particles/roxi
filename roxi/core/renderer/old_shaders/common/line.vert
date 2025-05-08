// from HelloVulkan
#version 460 core

#include <camera_ubo.glsl>

struct PointColor
{
	vec4 position;
	vec4 color;
};

layout(location = 0) out vec4 line_color;

layout(set = 0, binding = 0) uniform CameraBlock { CameraUBO cam_ubo; };
layout(set = 0, binding = 1) readonly buffer PointBlock { PointColor points[]; };

void main()
{
	PointColor pc = points[gl_VertexIndex];
	gl_Position = cam_ubo.projection * cam_ubo.view * pc.position;
	line_color = pc.color;
}
