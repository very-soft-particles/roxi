
#ifndef PI
#define PI 3.1415926535897932384626433832795
#endif

// Roughness remapping for IBL lighting
float alpha_ibl(float roughness)
{
	float alpha = (roughness * roughness) / 2.0;
	return alpha;
}

// Used to create Specular/Prefilter Map and BRDF LUT
vec3 importance_sample_ggx(vec2 xi, vec3 n, float roughness)
{
	float a = roughness * roughness; // Roughness remapping

	float phi = 2.0 * PI * xi.x;
    
    // TODO Potential issue where sqrt of negative number is undefined
	float cos_theta = sqrt((1.0 - Xi.y) / (1.0 + (a * a - 1.0) * Xi.y));
	float sin_theta = sqrt(1.0 - cos_theta * cos_theta);

	// From spherical coordinates to cartesian coordinates - halfway vector
	vec3 h;
	h.x = cos(phi) * sin_theta;
	h.y = sin(phi) * sin_theta;
	h.z = cos_theta;

	// From tangent-space H vector to world-space sample vector
	vec3 up = abs(n.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
	vec3 tangent = normalize(cross(up, n));
	vec3 bi_tangent = cross(n, tangent);

	vec3 sample_vec = tangent * h.x + bi_tangent * h.y + n * h.z;
	return normalize(sample_vec);
}
