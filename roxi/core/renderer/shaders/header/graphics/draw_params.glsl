struct DrawParameters {
  uint camera_id;
  uint vertex_buffer_id;
  uint index_buffer_id;
  uint mesh_buffer_id;

  uint material_buffer_id;
  uint model_buffer_id;
  uint instance_buffer_id;
  uint light_buffer_id;

  uint light_cell_buffer_id;
  uint light_index_buffer_id;
  uint clustered_forward_params_buffer_id;
  uint aabb_buffer_id;
};
