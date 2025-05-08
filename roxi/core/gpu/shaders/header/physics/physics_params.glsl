struct PhysicsParameters {
  uint in_velocity_buffer_id;
  uint out_velocity_buffer_id;
  uint in_transform_buffer_id;
  uint out_transform_buffer_id;

  uint desired_directions_buffer_id;
  uint acceleration_buffer_id;
  uint partition_buffer_id;
  uint sphere_buffer_id;

  uint collision_buffer_id;
  uint camera_id;
  uint object_bound_buffer_id;
  uint _pad0;
};
