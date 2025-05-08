
/*
Notations
	V	View unit vector
	L	Incident light unit vector
	N	Surface normal unit vector
	H	Half unit vector between L and V
*/

#ifndef PI
#define PI 3.1415926535897932384626433832795
#endif

// Specular D
// Trowbridge-Reitz GGX that models the distribution of microfacet normal.
float distribution_ggx(float noh, float roughness)
{
	float alpha = roughness * roughness; // Disney remapping
	float alpha2 = alpha * alpha;
	float noh2 = noh * noh;

	float nominator = alpha2;
	float denominator = (noh2 * (alpha2 - 1.0) + 1.0);
	denominator = PI * denominator * denominator;

	return nominator / denominator;
}

// Roughness remapping for direct lighting (See Brian Karis's PBR Note)
float alpha_direct_lighting(float roughness)
{
	float r = (roughness + 1.0);
	float alpha = (r * r) / 8.0;
	return alpha;
}

// Specular G
// Geometry function that describes the self-shadowing property of the microfacets.
// When a surface is relatively rough, the surface's microfacets can overshadow other
// microfacets reducing the light the surface reflects.
float geometry_schlick_ggx(float nol, float nov, float alpha)
{
	float gl = nol / (nol * (1.0 - alpha) + alpha);
	float gv = nov / (nov * (1.0 - alpha) + alpha);
	return gl * gv;
}

// Specular F
// The Fresnel equation describes the ratio of surface reflection at different surface angles.
vec3 fresnel_schlick(float cos_theta, vec3 f0)
{
	return f0 + (1.0 - f0) * pow(clamp(1.0 - cos_theta, 0.0, 1.0), 5.0);
}

vec3 fresnel_schlick_roughness(float cos_theta, vec3 f0, float roughness)
{
	return f0 + (max(vec3(1.0 - roughness), f0) - f0) * pow(clamp(1.0 - cos_theta, 0.0, 1.0), 5.0);
}

// Basic Lambertian diffuse
vec3 diffuse(vec3 albedo)
{
	return albedo / PI;
}
