#include "common.glsl"
#include "physics/physics_params.glsl"
#include "aabb/aabb.glsl"


REGISTER_UNIFORM
  ( physics_parameters
  , {
      PhysicsParameters data;
    }
  );

REGISTER_BUFFER
  ( std430
  , writeonly
  , partitions
  , {
      AABB data[];
    }
  );
#define get_partitions()  \
  GET_RESOURCE(partitions,       \
    g_physics_parameters[param_id].data.partition_buffer_id).data\
  )


