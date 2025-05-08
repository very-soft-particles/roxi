#include "physics/physics_params.glsl"


REGISTER_UNIFORM
  ( physics_parameters
  , {
      PhysicsParameters data;
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
#define get_sphere_radii_buffer()     \
    GET_RESOURCE(sphere_radii,               \
      g_physics_parameters[param_id].data.sphere_radii_buffer_id).data \
  )

REGISTER_BUFFER
  ( std430
  , readonly
  , desired_directions
  , {
      vec3 data[];
    }
  );
#define get_desired_directions()  \
  GET_RESOURCE(desired_directions,       \
    g_physics_parameters[param_id].data.desired_directions_buffer_id).data\
  )


REGISTER_BUFFER
  ( std430
  , readonly
  , positions_in
  , {
      vec3 data[];
    }
  );
#define get_positions()  \
  GET_RESOURCE(positions_in,       \
    g_physics_parameters[param_id].data.position_in_buffer_id).data\
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
    g_physics_parameters[param_id].data.position_out_buffer_id).data\
  )


REGISTER_BUFFER
  ( std430
  , readonly
  , velocities_in
  , {
      vec3 data[];
    }
  );
#define get_velocities_in()  \
  GET_RESOURCE(velocities_in,       \
    g_physics_parameters[param_id].data.velocities_in_buffer_id).data\
  )

REGISTER_BUFFER
  ( std430
  , writeonly
  , velocities_out
  , {
      vec3 data[];
    }
  );
#define get_velocities_out()  \
  GET_RESOURCE(velocities_out,       \
    g_physics_parameters[param_id].data.velocities_out_buffer_id).data\
  )

REGISTER_BUFFER
  ( std430
  , readonly
  , accelerations_in
  , {
      vec3 data[];
    }
  );
#define get_accelerations_in()  \
  GET_RESOURCE(accelerations_in,       \
    g_sphere_parameters[param_id].data.accelerations_in_buffer_id).data\
  )

REGISTER_BUFFER
  ( std430
  , writeonly
  , accelerations_out
  , {
      vec3 data[];
    }
  );
#define get_accelerations_out()  \
  GET_RESOURCE(accelerations_out,       \
    g_sphere_parameters[param_id].data.accelerations_out_buffer_id).data\
  )
