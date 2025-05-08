#version 460 core
#extension GL_EXT_buffer_reference : require

#include <model_ubo.glsl>
#include <shadow_mapping/ubo.glsl>
#include <bindless/vertex_data.glsl>
#include <bindless/mesh_data.glsl>
#include <bindless/bda.glsl>

layout(push_constant) uniform PC { BDA bda; };

layout(set = 0, binding = 0) uniform UBOBlock { ShadowUBO shadow_ubo; };
layout(set = 0, binding = 1) readonly buffer ModelUBOs { ModelUBO model_ubos []; };

void main()
{
	MeshData mesh_data = bda.meshReference.meshes[gl_BaseInstance];
	uint vert_offset = mesh_data.vertex_offset;
	uint index_offset = mesh_data.index_offset;
	uint vert_index = bda.index_reference.indices[index_offset + gl_VertexIndex] + vert_offset;
	VertexData vertex_data = bda.vertex_reference.vertices[vert_index];
	mat4 model = model_ubos[mesh_data.model_matrix_index].model;

	// Output
	gl_Position = shadow_ubo.light_space_matrix * model * vec4(vertex_data.position, 1.0);
}
