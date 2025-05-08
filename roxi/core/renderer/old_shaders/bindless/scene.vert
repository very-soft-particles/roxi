#version 460 core
#extension GL_EXT_buffer_reference : require

// bindless/scene.vert
// bindless textures

#include <model_ubo.glsl>
#include <camera_ubo.glsl>
#include <bindless/vertex_data.glsl>
#include <bindless/mesh_data.glsl>
#include <bindless/bda.glsl>

layout(location = 0) out vec3 world_pos;
layout(location = 1) out vec2 tex_coord;
layout(location = 2) out vec3 normal;
layout(location = 3) out vec3 vertex_color;
layout(location = 4) out flat uint mesh_idx;

layout(set = 0, binding = 0) uniform CameraBlock { CameraUBO cam_ubo; }; // UBO
layout(set = 0, binding = 1) readonly buffer ModelUBOs { ModelUBO model_ubos[]; }; // SSBO
layout(set = 0, binding = 2) uniform BDABlock { BDA bda; }; // UBO

void main()
{
	MeshData mesh_data = bda.mesh_reference.meshes[gl_BaseInstance];
	uint vert_offset = mesh_data.vertex_offset;
	uint index_offset = mesh_data.index_offset;
	uint vert_idx = bda.index_reference.indices[index_offset + gl_VertexIndex] + vert_offset;
	VertexData vertex_data = bda.vertex_reference.vertices[vert_idx];
	mat4 model = model_ubos[mesh_data.model_matrix_index].model;
	mat3 normal_matrix = transpose(inverse(mat3(model)));

	// Output
	world_pos = (model * vec4(vertex_data.position, 1.0)).xyz;
	tex_coord = vec2(vertex_data.uvx, vertex_data.uvy);
	normal = normal_matrix * vertex_data.normal;
	vertex_color = vertex_data.color.xyz;
	mesh_idx = gl_BaseInstance;
	gl_Position =  cam_ubo.projection * cam_ubo.view * vec4(world_pos, 1.0);
}
