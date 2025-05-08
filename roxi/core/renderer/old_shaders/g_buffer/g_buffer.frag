#version 460 core
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_buffer_reference : require

#include <camera_ubo.glsl>
#include <bindless/vertex_data.glsl>
#include <bindless/mesh_data.glsl>
#include <bindless/bda.glsl>

layout(location = 0) in vec3 view_pos;
layout(location = 1) in vec3 frag_pos;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 tex_coord;
layout(location = 4) in flat uint mesh_idx;

layout(location = 0) out vec4 g_position;
layout(location = 1) out vec3 g_normal;

layout(set = 0, binding = 0) uniform CameraBlock { CameraUBO cam_ubo; }; // UBO
layout(set = 0, binding = 2) uniform BDABlock { BDA bda; }; // UBO
layout(set = 0, binding = 3) uniform sampler2D pbr_textures[];

float linear_depth(float z, float near, float far)
{
	return near * far / (far + z * (near - far));
}

void main()
{
	MeshData mesh_data = bda.mesh_reference.meshes[mesh_idx];
	float alpha = texture(pbr_textures[nonuniformEXT(mesh_data.albedo)], tex_coord).a;
	if (alpha < 0.5)
	{
		discard;
	}

	// TODO gl_FragCoord.z is always 1.0 for some reason
	g_position = vec4(view_pos, linear_depth(frag_pos.z, cam_ubo.camera_near, cam_ubo.camera_far));
	g_normal = normalize(normal) * 0.5 + 0.5;
}
