// from HelloVulkan
#version 460 core
#extension GL_ARB_separate_shader_objects : enable

#include <camera_ubo.glsl>
#include <light_data.glsl>

layout(location = 0) out vec2 frag_offset;
layout(location = 1) out vec4 circle_color;

layout(set = 0, binding = 0) uniform CameraBlock { CameraUBO cam_ubo; };
layout(binding = 1) readonly buffer Lights { LightData lights []; };

const vec2 OFFSETS[6] = vec2[](
	vec2(-1.0, -1.0),
	vec2(-1.0,  1.0),
	vec2( 1.0, -1.0),
	vec2( 1.0, -1.0),
	vec2(-1.0,  1.0),
	vec2( 1.0,  1.0)
);
const float RADIUS = 0.1;

void main()
{
	LightData light = lights[gl_InstanceIndex];

	vec3 cam_right = vec3(cam_ubo.view[0][0], cam_ubo.view[1][0], cam_ubo.view[2][0]);
	vec3 cam_up = vec3(cam_ubo.view[0][1], cam_ubo.view[1][1], cam_ubo.view[2][1]);

	frag_offset = OFFSETS[gl_VertexIndex];

	vec3 position_world = light.position.xyz +
		RADIUS * frag_offset.x * cam_right +
		RADIUS * frag_offset.y * cam_up;

	circle_color = light.color;
	gl_Position = cam_ubo.projection * cam_ubo.view * vec4(position_world, 1.0);
}
