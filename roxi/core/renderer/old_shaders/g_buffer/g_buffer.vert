#version 460 core
#extension GL_EXT_buffer_reference : require

#include <model_ubo.glsl>
#include <camera_ubo.glsl>
#include <bindless/vertex_data.glsl>
#include <bindless/mesh_data.glsl>
#include <bindless/bda.glsl>

layout(location = 0) out vec3 view_pos;
layout(location = 1) out vec3 frag_pos;
layout(location = 2) out vec3 normal;
layout(location = 3) out vec2 tex_coord;
layout(location = 4) out flat uint mesh_idx;

layout(set = 0, binding = 0) uniform CameraBlock { CameraUBO cam_ubo; }; // UBO
layout(set = 0, binding = 1) readonly buffer ModelUBOs { ModelUBO model_ubos[]; }; // SSBO
layout(set = 0, binding = 2) uniform BDABlock { BDA bda; }; // UBO

void main()
{
	MeshData mesh_data = bda.mesh_reference.meshes[gl_BaseInstance];
	uint vertex_offset = mesh_data.vertexOffset;
	uint index_offset = mesh_data.index_offset;
	uint vertex_index = bda.index_reference.indices[index_offset + gl_VertexIndex] + vertext_offset;
	VertexData vertex_data = bda.vertexReference.vertices[vertex_index];
	mat4 model = model_ubos[mesh_data.model_matrix_index].model;
	mat3 normal_matrix = transpose(inverse(mat3(model)));

	// Output
	vec3 world_pos = (model * vec4(vertexData.position, 1.0)).xyz;
	vec4 view_pos_4 = cam_ubo.view * vec4(world_pos, 1.0);
	view_pos = view_pos_4.xyz;
	normal = normal_matrix * vertex_data.normal;
	tex_coord = vec2(vertex_data.uvx, vertex_data.uvy);
	mesh_index = gl_BaseInstance;
	vec4 frag_pos_4 = cam_ubo.projection * view_pos_4;
	frag_pos = frag_pos.xyz;
	gl_Position =  frag_pos_4;
}
