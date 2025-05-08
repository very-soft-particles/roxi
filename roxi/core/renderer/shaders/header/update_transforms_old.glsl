#include "common.glsl"
#include "physics/desired_direction.glsl"
REGISTER_UNIFORM
  ( physics_parameters
  , {
      PhysicsParameters data;
    }
  );

REGISTER_BUFFER
  ( std430
  , readonly
  , desired_directions
  , {
      DesiredDir data[];
    }
  );
#define get_desired_directions()  \
  GET_RESOURCE(desired_directions,       \
    g_physics_parameters[param_id].data.desired_directions_buffer_id).data\
  )

REGISTER_BUFFER
  ( std430
  , readonly
  , transforms0
  , {
      mat4 data[];
    }
  );
#define get_transforms0()  \
  GET_RESOURCE(transforms0,       \
    g_physics_parameters[param_id].data.in_transform_buffer_id).data\
  )

REGISTER_BUFFER
  ( std430
  , writeonly
  , transforms1
  , {
      mat4 data[];
    }
  );
#define get_transforms1()  \
  GET_RESOURCE(transforms1,       \
    g_physics_parameters[param_id].data.out_transform_buffer_id).data\
  )

REGISTER_BUFFER
  ( std430
  , readonly
  , velocities0
  , {
      float data[];
    }
  );
#define get_velocities0()  \
  GET_RESOURCE(velocities0,       \
    g_physics_parameters[param_id].data.in_velocity_buffer_id).data\
  )

REGISTER_BUFFER
  ( std430
  , writeonly
  , velocities1
  , {
      float data[];
    }
  );
#define get_velocities1()  \
  GET_RESOURCE(velocities1,       \
    g_physics_parameters[param_id].data.out_velocity_buffer_id).data\
  )

REGISTER_BUFFER
  ( std430
  , readonly
  , accelerations
  , {
      float data[];
    }
  );
#define get_accelerations()  \
  GET_RESOURCE(accelerations,       \
    g_physics_parameters[param_id].data.acceleration_buffer_id).data\
  )

