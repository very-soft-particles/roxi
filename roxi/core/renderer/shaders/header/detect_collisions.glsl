#include "physics/collision.glsl"
#include "physics/detect_collision_params.glsl"
#include "physics/sphere.glsl"


REGISTER_UNIFORM
  ( collision_parameters
  , {
      DetectCollisionParameters data;
    }
  );

REGISTER_BUFFER
  ( std430
  , readonly
  , sphere_radii
  , {
      float data[];
    }
  );
#define get_sphere_radii()     \
    GET_RESOURCE(sphere_radii,               \
      g_detect_collision_parameters[param_id].data.sphere_radii_buffer_id).data \
  )

REGISTER_BUFFER
  ( std430
  , readonly
  , obj_indices
  , {
      uint data[];
    }
  );
#define get_obj_indices()  \
  GET_RESOURCE(obj_indices,       \
    g_detect_collision_parameters[param_id].data.position_in_buffer_id).data\
  )

REGISTER_BUFFER
  ( std430
  , readonly
  , bucket_infos
  , {
      BucketInfo data[];
    }
  );
#define get_bucket_infos()  \
  GET_RESOURCE(bucket_infos,       \
    g_detect_collision_parameters[param_id].data.bucket_info_buffer_id).data\
  )



REGISTER_BUFFER
  ( std430
  , readonly
  , positions_in
  , {
      vec3 data[];
    }
  );
#define get_positions_in()  \
  GET_RESOURCE(positions_in,       \
    g_detect_collision_parameters[param_id].data.position_in_buffer_id).data\
  )

REGISTER_BUFFER
  ( std430
  , writeonly
  , collisions_out
  , {
      Collision data[];
    }
  );
#define get_collisions_out()  \
  GET_RESOURCE(collisions_out,       \
    g_detect_collision_parameters[param_id].data.collisions_out_buffer_id).data\
  )

REGISTER_UNIFORM
  ( total_collision_counter
  , {
      uint data;
    }
  );
#define get_collision_counter()  \
  GET_RESOURCE(collision_counter,       \
    g_sphere_parameters[param_id].data.collision_count_buffer_id).data\
  )
