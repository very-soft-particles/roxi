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
	vec3 pre_filtered_color = textureLod(specular_map, r, roughness * pc.max_reflection_lod).rgb;
	vec2 brdf = texture(brdf_lut, vec2(nov, roughness)).rg;
	vec3 specular = pre_filtered_color * (f * brdf.x + brdf.y);

	return (kd * diffuse + specular) * ao;
}
