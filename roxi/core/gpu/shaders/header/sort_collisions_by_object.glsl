#include "physics/collision.glsl"
#include "physics/sort_collision_params.glsl"
#include "physics/sphere.glsl"


REGISTER_UNIFORM
  ( sort_collision_parameters
  , {
      SortCollisionParameters data;
    }
  );

REGISTER_BUFFER
  ( std430
  , readonly
  , collisions_in
  , {
      Collision data[];
    }
  );
#define get_collisions_in()  \
  GET_RESOURCE(collisions_in,       \
    g_sort_collision_parameters[param_id].data.collisions_in_buffer_id).data\
  )

REGISTER_BUFFER
  ( std430
  , writeonly
  , collisions_per_objs
  , {
      uint data[];
    }
  );
#define get_collisions_per_obj()  \
  GET_RESOURCE(collisions_per_objs,       \
    g_detect_collision_parameters[param_id].data.collisions_per_obj_buffer_id).data\
  )

REGISTER_BUFFER
  ( std430
  , writeonly
  , collisions_per_objs
  , {
      uint data[];
    }
  );
#define get_collisions_per_obj()  \
  GET_RESOURCE(collisions_per_objs,       \
    g_detect_collision_parameters[param_id].data.collisions_per_obj_buffer_id).data\
  )

