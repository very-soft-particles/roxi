
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
