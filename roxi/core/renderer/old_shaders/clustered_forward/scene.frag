// from HelloVulkan
#version 460 core
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_buffer_reference : require

//layout(early_fragment_tests) in;

/*
ClusteredForward/Scene.frag 

Fragment shader for PBR+IBL, naive forward shading (non clustered)
*/

// Include files
#include <camera_ubo.glsl>
#include <light_data.glsl>
#include <pbr/pbr_header.glsl>
#include <pbr/pbr_push_constants.glsl>
#include <pbr/hammersley.glsl>
#include <pbr/normal_tbn.glsl>
#include <clustered_forward/header.glsl>
#include <bindless/vertex_data.glsl>
#include <bindless/mesh_data.glsl>
#include <bindless/bda.glsl>

// Specialization constant
layout (constant_id = 0) const uint ALPHA_DISCARD = 1;

layout(location = 0) in vec3 world_pos;
layout(location = 1) in vec2 tex_coord;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec3 vertex_color;
layout(location = 4) in flat uint mesh_index;

layout(location = 0) out vec4 frag_color;

layout(push_constant) uniform PC { PBRPushConstant pc; };

layout(set = 0, binding = 0) uniform CameraBlock { CameraUBO cam_ubo; }; // UBO
layout(set = 0, binding = 2) uniform BDABlock { BDA bda; }; // UBO
layout(set = 0, binding = 3) uniform CFUBO { ClusterForwardUBO cf_ubo; }; // UBO
layout(set = 0, binding = 4) readonly buffer Lights { LightData lights []; }; // SSBO
layout(set = 0, binding = 5) readonly buffer LightCells { LightCell light_cells []; }; // SSBO
layout(set = 0, binding = 6) readonly buffer LightIndices { uint light_indices []; }; // SSBO

layout(set = 0, binding = 7) uniform samplerCube specular_map;
layout(set = 0, binding = 8) uniform samplerCube diffuse_map;
layout(set = 0, binding = 9) uniform sampler2D brdf_lut;

layout(set = 0, binding = 10) uniform sampler2D pbr_textures[] ;

#include <clustered_forward/radiance.glsl>
#include <pbr/ambient.glsl>

// stackoverflow.com/questions/51108596/linearize-depth
float linear_depth(float z, float near, float far)
{
	return near * far / (far + z * (near - far));
}

void main()
{
	// Clustered Forward
	// Must use GLM_FORCE_DEPTH_ZERO_TO_ONE preprocessor, or the code below will fail
	float lin_depth = linear_depth(gl_FragCoord.z, cf_ubo.camera_near, cf_ubo.camera_far);
	uint z_index = uint(max(log2(lin_depth) * cf_ubo.slice_scaling + cf_ubo.slice_bias, 0.0));
	vec2 tile_size =
		vec2(cf_ubo.screen_size.x / float(cf_ubo.slice_count_x),
			 cf_ubo.screen_size.y / float(cf_ubo.slice_count_y));
	uvec3 cluster = uvec3(
		gl_FragCoord.x / tile_size.x,
		gl_FragCoord.y / tile_size.y,
		z_index);
	uint cluster_idx =
		cluster.x +
		cluster.y * cf_ubo.slice_count_x +
		cluster.z * cf_ubo.slice_count_x * cf_ubo.slice_count_y;
	uint light_count = light_cells[cluster_idx].count;
	uint light_idx_offset = light_cells[cluster_idx].offset;

	// Buffer device address
	MeshData mesh_data = bda.mesh_reference.meshes[mesh_index];

	// PBR + IBL
	vec4 albedo4 = texture(pbr_textures[nonuniformEXT(mesh_data.albedo)], tex_coord).rgba;

	// A performance trick by using a use specialization constant
	// so this part will be removed if material type is not transparent
	if (ALPHA_DISCARD > 0)
	{
		if (albedo4.a < 0.5)
		{
			discard;
		}
	}

	// PBR + IBL, Material properties
	vec3 albedo = pow(albedo4.rgb, vec3(2.2)); // Conversion from SRGB (gamma corrected) to UNORM (linear)
	vec3 emissive = texture(pbr_textures[nonuniformEXT(mesh_data.emissive)], tex_coord).rgb;
	vec3 tex_normal_value = texture(pbr_textures[nonuniformEXT(mesh_data.normal)], tex_coord).xyz * 2.0 - 1.0;
	float metallic = texture(pbr_textures[nonuniformEXT(mesh_data.metalness)], tex_coord).b;
	float roughness = texture(pbr_textures[nonuniformEXT(mesh_data.roughness)], tex_coord).g;
	float ao = texture(pbr_textures[nonuniformEXT(mesh_data.ao)], tex_coord).r;
	float alpha_roughness = alpha_direct_lighting(roughness);

	// Input lighting data
	vec3 n = normal_tbn(tex_normal_value, world_pos, normal, tex_coord);
	vec3 v = normalize(cam_ubo.position.xyz - world_pos);
	float nov = max(dot(n, v), 0.0);

	// Calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 
	// of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)  
	vec3 f0 = vec3(pc.base_reflectivity);
	f0 = mix(f0, albedo, metallic);

	// A little bit hacky
	//vec3 Lo = vec3(0.0); // Original code
	vec3 lo = albedo * pc.albedo_multipler;

	for (int i = 0; i < light_count; ++i)
	{
		uint light_idx = light_indices[i + light_idx_offset];
		LightData light = lights[light_idx];

		lo += radiance(
			albedo,
			n,
			v,
			f0,
			metallic,
			roughness,
			alpha_roughness,
			nov,
			light);
	}

	vec3 ambient = ambient(
		albedo,
		f0,
		n,
		v,
		metallic,
		roughness,
		ao,
		nov);

	vec3 color = ambient + emissive + lo;

	frag_color = vec4(color, 1.0);
}
