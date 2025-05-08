#version 460 core
#extension GL_EXT_buffer_reference : require

/*
// ShadowMapping/Scene.vert  

Vertex shader for 
	* PBR+IBL
	* Naive forward shading (non clustered)
	* Shadow mapping
	* Bindless textures
	* Buffer device address
*/

#include <camera_ubo.glsl>
#include <model_ubo.glsl>
#include <shadow_mapping/ubo.glsl>
#include <bindless/vertex_data.glsl>
#include <bindless/mesh_data.glsl>
#include <bindless/bda.glsl>

layout(location = 0) out vec3 world_pos;
layout(location = 1) out vec2 tex_coord;
layout(location = 2) out vec3 normal;
layout(location = 3) out vec4 shadow_pos;
layout(location = 4) out vec4 frag_pos;
layout(location = 5) out flat uint mesh_index;

layout(set = 0, binding = 0) uniform CameraBlock { CameraUBO cam_ubo; }; // UBO
layout(set = 0, binding = 1) uniform UBOBlock { ShadowUBO shadow_ubo; }; // UBO
layout(set = 0, binding = 2) readonly buffer ModelUBOs { ModelUBO model_ubos []; }; // SSBO

// Buffer device address
layout(set = 0, binding = 3) uniform BDABlock { BDA bda; }; // UBO

const mat4 biasMat = mat4(
	0.5, 0.0, 0.0, 0.0,
	0.0, 0.5, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.5, 0.5, 0.0, 1.0);

void main()
{
	MeshData mesh_data = bda.mesh_reference.meshes[gl_BaseInstance];
	uint vert_offset = mesh_data.vertex_offset;
	uint index_offset = mesh_data.index_offset;
	uint vert_index = bda.index_reference.indices[index_offset + gl_VertexIndex] + vert_offset;
	VertexData vertex_data = bda.vertex_reference.vertices[vert_index];
	mat4 model = model_ubos[mesh_data.model_matrix_index].model;
	mat3 normal_matrix = transpose(inverse(mat3(model)));

	// Output
	vec4 position4 = vec4(vertex_data.position,1.0);
	world_pos = (model * position4).xyz;
	tex_coord = vec2(vertex_data.uvX, vertex_data.uvY);
	normal = normalMatrix * vertex_data.normal;
	mesh_index = gl_BaseInstance;
	shadow_pos = bias_mat * shadow_ubo.light_space_matrix * model * position4;
	frag_pos = cam_ubo.projection * cam_ubo.view * vec4(world_pos, 1.0);
	gl_Position = frag_pos;
}
