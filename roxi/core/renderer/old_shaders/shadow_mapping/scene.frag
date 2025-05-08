#version 460 core
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_buffer_reference : require

// www.khronos.org/opengl/wiki/Early_Fragment_Test 
//layout(early_fragment_tests) in;

/*
ShadowMapping/Scene.frag 

Fragment shader for 
	* PBR+IBL
	* Naive forward shading (non clustered)
	* Shadow mapping
	* Bindless textures
	* Buffer device address
*/

// Include files
#include <camera_ubo.glsl>
#include <light_data.glsl>
#include <pbr/pbr_header.glsl>
#include <pbr/pbr_pushconstants.glsl>
#include <pbr/hammersley.glsl>
#include <pbr/normal_tbn.glsl>
#include <bindless/vertex_data.glsl>
#include <bindless/mesh_data.glsl>
#include <shadow_mapping/ubo.glsl>
#include <bindless/bda.glsl>

// Specialization constant
layout (constant_id = 0) const uint ALPHA_DISCARD = 1;

layout(location = 0) in vec3 world_pos;
layout(location = 1) in vec2 tex_coord;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec4 shadow_pos;
layout(location = 4) in vec4 frag_pos;
layout(location = 5) in flat uint mesh_index;

layout(location = 0) out vec4 frag_color;

layout(push_constant) uniform PC { PBRPushConstant pc; };

layout(set = 0, binding = 0) uniform CameraBlock { CameraUBO cam_ubo; }; // UBO
layout(set = 0, binding = 1) uniform UBOBlock { ShadowUBO shadow_ubo; }; // UBO
layout(set = 0, binding = 3) uniform BDABlock { BDA bda; };
layout(set = 0, binding = 4) readonly buffer Lights { LightData lights []; }; // SSBO
layout(set = 0, binding = 5) uniform samplerCube specular_map;
layout(set = 0, binding = 6) uniform samplerCube diffuse_map;
layout(set = 0, binding = 7) uniform sampler2D brdf_lut;
layout(set = 0, binding = 8) uniform sampler2D shadow_map;
layout(set = 0, binding = 9) uniform sampler2D ssao_tex;

// NOTE This requires descriptor indexing feature
layout(set = 0, binding = 10) uniform sampler2D pbr_textures[];

// PCF or Poisson
#include <shadow_mapping/shadow.glsl>

// PBR and IBL
#include <pbr/radiance.glsl>
#include <pbr/ambient.glsl>

void main()
{
	// This uses buffer device address
	MeshData mesh_data = bda.mesh_reference.meshes[mesh_index];

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

	// Material properties
	vec3 albedo = pow(albedo4.rgb, vec3(2.2)); // Conversion from SRGB (gamma corrected) to UNORM (linear)
	vec3 emissive = texture(pbr_textures[nonuniformEXT(mesh_data.emissive)], tex_coord).rgb;
	vec3 tex_normal_value = texture(pbr_textures[nonuniformEXT(mesh_data.normal)], tex_coord).xyz * 2.0 - 1.0;
	float metallic = texture(pbr_textures[nonuniformEXT(mesh_data.metalness)], tex_coord).b;
	float roughness = texture(pbr_textures[nonuniformEXT(mesh_data.roughness)], tex_coord).g;
	float ao = texture(pbr_textures[nonuniformEXT(mesh_data.ao)], tex_coord).r;
	float alpha_roughness = alpha_direct_lighting(roughness);

	// SSAO
	vec2 frag_coord = (frag_pos.xy / frag_pos.w);
	vec2 screen_coord = frag_coord * 0.5 + 0.5;
	float ssao = texture(ssao_tex, screen_coord).r;

	vec3 n = normal_tbn(tex_normal_value, world_pos, normal, tex_coord);
	vec3 v = normalize(cam_ubo.position.xyz - world_pos);
	float nov = max(dot(n, v), 0.0);

	// Calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 
	// of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)  
	vec3 f0 = vec3(pc.base_reflectivity);
	f0 = mix(f0, albedo, metallic);

	// A little bit hacky
	vec3 lo =  albedo * pc.albedo_multipler;

	for (int i = 0; i < lights.length(); ++i)
	{
		LightData light = lights[i];

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

	float shadow = shadow_poisson(shadow_pos / shadow_pos.w);
	vec3 color = ambient + emissive + (lo * shadow);
	color *= ssao;
	frag_color = vec4(color, 1.0);
}
