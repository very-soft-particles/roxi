const int POISSON_COUNT = 16;

const vec2 POISSON_DISK[POISSON_COUNT] = vec2[](
	vec2(-0.94201624, -0.39906216),
	vec2(0.94558609, -0.76890725),
	vec2(-0.094184101, -0.92938870),
	vec2(0.34495938, 0.29387760),
	vec2(-0.91588581, 0.45771432),
	vec2(-0.81544232, -0.87912464),
	vec2(-0.38277543, 0.27676845),
	vec2(0.97484398, 0.75648379),
	vec2(0.44323325, -0.97511554),
	vec2(0.53742981, -0.47373420),
	vec2(-0.26496911, -0.41893023),
	vec2(0.79197514, 0.19090188),
	vec2(-0.24188840, 0.99706507),
	vec2(-0.81409955, 0.91437590),
	vec2(0.19984126, 0.78641367),
	vec2(0.14383161, -0.14100790)
);
// Returns a pseudo random number based on a vec3 and an int.
float rand(vec3 seed, int i)
{
	vec4 seed4 = vec4(seed, i);
	float dot_value = dot(seed4, vec4(12.9898, 78.233, 45.164, 94.673));
	return fract(sin(dot_value) * 43758.5453);
}

vec2 get_poisson_disk_coord(vec2 proj_coords, int i, float radius)
{
	int index = int(float(POISSON_COUNT) * rand(proj_coords.xyy, i)) % POISSON_COUNT;
	return proj_coords.xy + POISSON_DISK[index] / radius;
}
