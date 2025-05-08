// Non-clustered version
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

	// Physically correct attenuation
	//float attenuation = 1.0 / (distance * distance);

	// Hacky attenuation
	float attenuation = 1.0 / pow(distance, pc.light_falloff);

	vec3 radiance = light.color.xyz * attenuation * pc.light_intensity;

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
