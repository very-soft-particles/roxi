#version 460 core
#include "../header/graphics.glsl"
#include "../spec_constants/draw_param_spec_constant.glsl"

layout(location = 0) in vec3 world_pos;
layout(location = 1) in vec2 tex_coord;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec3 vertex_color;
layout(location = 4) in flat uint material_index;

layout(location = 0) out vec4 frag_color;

vec3 radiance(
	vec3 albedo,
	vec3 n,
	vec3 v,
	vec3 f0,
	float metallic,
	float roughness,
	float alpha_roughness,
	float nov,
	LightData light)
{
	vec3 lo = vec3(0.0);

	vec3 l = normalize(light.position.xyz - world_pos); // Incident light vector
	vec3 h = normalize(v + l); // Halfway vector
	float noh = max(dot(n, h), 0.0);
	float nol = max(dot(n, l), 0.0);
	float hov = max(dot(h, v), 0.0);
	float distance = length(light.position.xyz - world_pos);

	// NOTE This is the only difference compared to the other Radiance.glsl
  // 1.0f final scalar is light falloff
	float attenuation = max(1.0 - (distance / light.radius), 0.0) / pow(distance, 1.0f);

	// Also, several attenuation formulas are proposed by Nikita Lisitsa:
	// lisyarus.github.io/blog/graphics/2022/07/30/point-light-attenuation.html

  // 1.0f final scalar is light intensity
	vec3 radiance = light.color.xyz * attenuation * 1.0f;

	// Cook-Torrance BRDF
	float d = distribution_ggx(noh, roughness);
	float g = geometry_schlick_ggx(nol, nov, alpha_roughness);
	vec3 f = fresnel_schlick(hov, f0);

	vec3 numerator = d * g * f;
	float denominator = 4.0 * nov * nol + 0.0001; // + 0.0001 to prevent divide by zero
	vec3 specular = numerator / denominator;

	// kS is equal to Fresnel
	vec3 ks = f;
	// For energy conservation, the diffuse and specular light can't
	// be above 1.0 (unless the surface emits light); to preserve this
	// relationship the diffuse component (kD) should equal 1.0 - kS.
	vec3 kd = vec3(1.0) - ks;
	// Multiply kD by the inverse metalness such that only non-metals 
	// have diffuse lighting, or a linear blend if partly metal (pure metals
	// have no diffuse light).
	kd *= 1.0 - metallic;

	vec3 diffuse = diffuse(kd * albedo);

	// Add to outgoing radiance Lo
	// Note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
	lo += (diffuse + specular) * radiance * nol;

	return lo;
}

vec3 ambient(
	vec3 albedo,
	vec3 f0,
	vec3 n,
	vec3 v,
	float metallic,
	float roughness,
	float ao,
	float nov)
{
	// Ambient lighting (we now use IBL as the ambient term)
	vec3 f = fresnel_schlick_roughness(nov, f0, roughness);

	vec3 ks = f;
	vec3 kd = 1.0 - ks;
	kd *= 1.0 - metallic;

	vec3 irradiance = texture(diffuse_map, n).rgb;
	vec3 diffuse = irradiance * albedo;

	// Sample both the pre-filter map and the BRDF lut and combine them together as
	// per the Split-Sum approximation to get the IBL specular part.
	vec3 r = reflect(-v, n);
  // final scalar 1.0f is max_lod
	vec3 pre_filtered_color = textureLod(specular_map, r, roughness * 1.0f).rgb;
	vec2 brdf = texture(brdf_lut, vec2(nov, roughness)).rg;
	vec3 specular = pre_filtered_color * (f * brdf.x + brdf.y);

	return (kd * diffuse + specular) * ao;
}
vec3 normal_tbn(vec3 texture_normal, vec3 world_pos, vec3 normal, vec2 tex_coord)
{
	vec3 q1 = dFdx(world_pos);
	vec3 q2 = dFdy(world_pos);
	vec2 st1 = dFdx(tex_coord);
	vec2 st2 = dFdy(tex_coord);

	vec3 n = normalize(normal);
	vec3 t = normalize(q1 * st2.y - q2 * st1.y);
	vec3 b = -normalize(cross(n, t));
	mat3 tbn = mat3(t, b, n);

	return normalize(tbn * texture_normal);
}

float linear_depth(float z, float near, float far)
{
	return near * far / (far + z * (near - far));
}

void main() {
  const uint param_id = GET_SPEC_CONSTANT(draw_param);

  ClusteredForwardData cf_data = get_clustered_forward_data(param_id);
  CameraData camera = get_camera(param_id);
	// Clustered Forward
	// Must use GLM_FORCE_DEPTH_ZERO_TO_ONE preprocessor, or the code below will fail
	float lin_depth = linear_depth(gl_FragCoord.z, camera.camera_near, camera.camera_far);
	uint z_index = uint(max(log2(lin_depth) * cf_data.slice_scaling + cf_data.slice_bias, 0.0));
	vec2 tile_size =
		vec2(cf_data.screen_size.x / float(cf_data.slice_count_x),
			 cf_data.screen_size.y / float(cf_data.slice_count_y));
	uvec3 cluster = uvec3(
		gl_FragCoord.x / tile_size.x,
		gl_FragCoord.y / tile_size.y,
		z_index);
	uint cluster_idx =
		cluster.x +
		cluster.y * cf_ubo.slice_count_x +
		cluster.z * cf_ubo.slice_count_x * cf_ubo.slice_count_y;
	uint light_count = get_light_cells(param_id)[cluster_idx].count;
	uint light_idx_offset = get_light_cells(param_id)[cluster_idx].offset;

	// Buffer device address
	MaterialData material_data = get_materials(param_id)[material_index];

	// PBR + IBL
	vec4 albedo4 = texture(pbr_textures[nonuniformEXT(material_data.albedo)], tex_coord).rgba;

	// A performance trick by using a use specialization constant
	// so this part will be removed if material type is not transparent
	if (GET_SPEC_CONSTANT(transparent) > 0)
	{
		if (albedo4.a < 0.5)
		{
			discard;
		}
	}

	// PBR + IBL, Material properties
	vec3 albedo = pow(albedo4.rgb, vec3(2.2)); // Conversion from SRGB (gamma corrected) to UNORM (linear)
	vec3 emissive = texture(pbr_textures[nonuniformEXT(material_data.emissive)], tex_coord).rgb;
	vec3 tex_normal_value = texture(pbr_textures[nonuniformEXT(material_data.normal)], tex_coord).xyz * 2.0 - 1.0;
	float metallic = texture(pbr_textures[nonuniformEXT(material_data.metalness)], tex_coord).b;
	float roughness = texture(pbr_textures[nonuniformEXT(material_data.roughness)], tex_coord).g;
	float ao = texture(pbr_textures[nonuniformEXT(material_data.ao)], tex_coord).r;
  float alpha_roughness = alpha_direct_lighting(roughness);

	// Input lighting data
	vec3 n = normal_tbn(tex_normal_value, world_pos, normal, tex_coord);
	vec3 v = normalize(cam_ubo.position.xyz - world_pos);
	float nov = max(dot(n, v), 0.0);

	// Calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 
	// of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)  
	vec3 f0 = vec3(0.04);
	f0 = mix(f0, albedo, metallic);

	// A little bit hacky
	vec3 lo = vec3(0.0); // Original code
	//vec3 lo = albedo * pc.albedo_multipler;

	for (int i = 0; i < light_count; ++i)
	{
		uint light_idx = get_light_indices(param_id)[i + light_idx_offset];
		LightData light = get_lights(param_id)[light_idx];

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
