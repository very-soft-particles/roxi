// for drawing a cube
const vec3 CUBE_POS[8] = vec3[8]
(
	vec3(-1.0, -1.0, 1.0),
	vec3(1.0, -1.0, 1.0),
	vec3(1.0, 1.0, 1.0),
	vec3(-1.0, 1.0, 1.0),

	vec3(-1.0, -1.0, -1.0),
	vec3(1.0, -1.0, -1.0),
	vec3(1.0, 1.0, -1.0),
	vec3(-1.0, 1.0, -1.0)
);

const int CUBE_INDICES[36] = int[36]
(
	// Front
	0, 1, 2, 2, 3, 0,
	// Right
	1, 5, 6, 6, 2, 1,
	// Back
	7, 6, 5, 5, 4, 7,
	// Left
	4, 0, 3, 3, 7, 4,
	// Bottom
	4, 5, 1, 1, 0, 4,
	// Top
	3, 2, 6, 6, 7, 3
);
