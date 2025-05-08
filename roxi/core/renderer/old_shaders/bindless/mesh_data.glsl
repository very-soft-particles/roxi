struct MeshData
{
	uint vertex_offset;
	uint index_offset;

	uint model_matrix_index;

	uint albedo;
	uint normal;
	uint metalness;
	uint roughness;
	uint ao;
	uint emissive;

	uint material;
};
