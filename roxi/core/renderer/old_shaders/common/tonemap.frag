// from HelloVulkan
#version 460 core

layout(location = 0) in vec2 tex_coord;
layout(location = 0) out vec4 frag_color;
layout(set = 0, binding = 0) uniform sampler2D color_image;

#define CLAMP01(x) clamp(x, 0.0, 1.0)

vec3 reinhard(vec3 color);
vec3 aces(vec3 x);
vec3 filmic(vec3 x);
vec3 partial_uncharted2(vec3 x);
vec3 uncharted2(vec3 v);
vec3 gamma_correction(vec3 color);

void main()
{
	vec3 color = texture(color_image, tex_coord).rgb;
	vec3 mapped_color = reinhard(color);
	mapped_color = gamma_correction(mapped_color);
	frag_color = vec4(mapped_color, 1.0);
}

vec3 reinhard(vec3 color)
{
	const float pure_white = 1.0;

	float luminance = dot(color, vec3(0.2126, 0.7152, 0.0722));
	float mapped_luminance =
		(luminance * (1.0 + luminance / (pure_white * pure_white))) / (1.0 + luminance);

	// Scale color by ratio of average luminances
	return (mapped_luminance / luminance) * color;
}

vec3 aces(vec3 x)
{
	const float a = 2.51;
	const float b = 0.03;
	const float c = 2.43;
	const float d = 0.59;
	const float e = 0.14;
	return CLAMP01((x * (a * x + b)) / (x * (c * x + d) + e));
}

vec3 filmic(vec3 x)
{
	vec3 X = max(vec3(0.0), x - 0.004);
	vec3 result = (X * (6.2 * X + 0.5)) / (X * (6.2 * X + 1.7) + 0.06);
	return pow(result, vec3(2.2));
}

vec3 partial_uncharted2(vec3 x)
{
	float A = 0.15;
	float B = 0.50;
	float C = 0.10;
	float D = 0.20;
	float E = 0.02;
	float F = 0.30;
	return ((x * (A * x + C * B) + D * E) / (x * (A * x + B) + D * F)) - E / F;
}

vec3 uncharted2(vec3 v)
{
	const float exposure_bias = 2.0;
	vec3 curr = partial_uncharted2(v * exposure_bias);

	vec3 W = vec3(11.2);
	const vec3 white_scale = vec3(1.0) / partial_uncharted2(W);
	return curr * white_scale;
}

vec3 gamma_correction(vec3 color)
{
	const float gamma = 2.2;
	return pow(color, vec3(1.0 / gamma));
}
