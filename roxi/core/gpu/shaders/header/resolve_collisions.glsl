#include "physics/collision.glsl"
#include "physics/collision_params.glsl"
#include "physics/sphere.glsl"


REGISTER_UNIFORM
  ( collision_parameters
  , {
      CollisionParameters data;
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
      g_physics_parameters[param_id].data.sphere_radii_buffer_id).data \
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
    g_collision_parameters[param_id].data.position_in_buffer_id).data\
  )

REGISTER_BUFFER
  ( std430
  , writeonly
  , positions_out
  , {
      vec3 data[];
    }
  );
#define get_positions_out()  \
  GET_RESOURCE(positions_out,       \
    g_collision_parameters[param_id].data.position_out_buffer_id).data\
  )

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
    g_collision_parameters[param_id].data.collisions_in_buffer_id).data\
  )

REGISTER_BUFFER
  ( std430
  , writeonly
  , collisions_out
  , {
      vec3 data[];
    }
  );
#define get_collisions_out()  \
  GET_RESOURCE(collisions_out,       \
    g_sphere_parameters[param_id].data.collisions_out_buffer_id).data\
  )
