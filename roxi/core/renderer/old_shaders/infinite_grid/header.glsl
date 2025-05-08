/*
Adapted from
	3D Graphics Rendering Cookbook
	by Sergey Kosarevsky & Viktor Latypov
	github.com/PacktPublishing/3D-Graphics-Rendering-Cookbook
*/

float log_10(float x)
{
	return log(x) / log(10.0);
}

float saturate_1(float x)
{
	return clamp(x, 0.0, 1.0);
}

vec2 saturate_2(vec2 x)
{
	return clamp(x, vec2(0.0), vec2(1.0));
}

float max_2(vec2 v)
{
	return max(v.x, v.y);
}

vec4 grid_color(vec2 uv, vec2 cam_pos)
{
	vec2 dudv = vec2(
		length(vec2(dFdx(uv.x), dFdy(uv.x))),
		length(vec2(dFdx(uv.y), dFdy(uv.y)))
	);

	float lod_level = max(0.0, log_10(GRID_CELL_SIZE) + 1.0);
	float lod_fade = fract(lod_level);

	// Cell sizes for lod0, lod1 and lod2
	float lod_0 = GRID_CELL_SIZE * pow(10.0, floor(lod_level));
	float lod_1 = lod_0 * 10.0;
	float lod_2 = lod_1 * 10.0;

	// Each anti-aliased line covers up to 4 pixels
	dudv *= 4.0;

	// Update grid coordinates for subsequent alpha calculations (centers each anti-aliased line)
	uv += dudv / 2.0F;

	// Calculate absolute distances to cell line centers for each lod and pick max X/Y to get coverage alpha value
	float lod_0a = max_2(vec2(1.0) - abs(saturate_2(mod(uv, lod_0) / dudv) * 2.0 - vec2(1.0)));
	float lod_1a = max_2(vec2(1.0) - abs(saturate_2(mod(uv, lod_1) / dudv) * 2.0 - vec2(1.0)));
	float lod_2a = max_2(vec2(1.0) - abs(saturate_2(mod(uv, lod_2) / dudv) * 2.0 - vec2(1.0)));

	uv -= cam_pos;

	// Blend between falloff colors to handle LOD transition
	vec4 c = lod_2a > 0.0 ? GRID_COLOR_THICK : lod_1a > 0.0 ? 
		mix(GRID_COLOR_THICK, GRID_COLOR_THIN, lod_fade) : 
		GRID_COLOR_THIN;

	// Calculate opacity falloff based on distance to grid extents
	float opacity_falloff = (1.0 - saturate_1(length(uv) / GRID_EXTENTS));

	// Blend between LOD level alphas and scale with opacity falloff
	c.a *= (lod_2a > 0.0 ? lod_2a : lod_1a > 0.0 ? lod_1a : (lod_0a * (1.0 - lod_fade))) * opacity_falloff;

	return c;
}
