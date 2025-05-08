#include "common.glsl"
#include "graphics/vertex_data.glsl"
#include "sphere/sphere_cull_info.glsl"
#include "aabb/aabb.glsl"
#include "aabb/generate_aabb_parameters.glsl"

REGISTER_UNIFORM
  ( generate_aabb_parameters
  , {
      GenerateAABBParameters data;
    }
  );

REGISTER_BUFFER
  ( std430
  , writeonly
  , aabbs
  , {
      AABB data[];
    }
  );
#define get_aabbs()  \
  GET_RESOURCE(aabbs,       \
    g_generate_aabb_parameters[param_id].data.aabb_buffer_id).data

REGISTER_BUFFER
  ( std430
  , readonly
  , vertices
  , {
      VertexData data[];
    }
  );
#define get_transforms()  \
  GET_RESOURCE(transforms,       \
    g_generate_aabb_parameters[param_id].data.vertex_buffer_id).data

REGISTER_BUFFER
  ( std430
  , 
  , atomic_max_buffer
  , {
      float data[];
    }
  );
#define get_atomic_max()\
  GET_RESOURCE(atomic_max_buffer,\
    g_generate_aabb_parameters[param_id].data.atomic_max_buffer_id).data

REGISTER_BUFFER
  ( std430
  , 
  , atomic_min_buffer
  , {
      float data[];
    }
  );
#define get_atomic_min()\
  GET_RESOURCE(atomic_min_buffer,\
    g_generate_aabb_parameters[param_id].data.atomic_min_buffer_id).data

REGISTER_BUFFER
  ( std430
  , writeonly
  , atomic_max_idx_buffer
  , {
      uint data[];
    }
  );
#define get_atomic_max_idx()\
  GET_RESOURCE(atomic_max_idx_buffer,\
    g_generate_aabb_parameters[param_id].data.atomic_max_idx_buffer_id).data

REGISTER_BUFFER
  ( std430
  , writeonly
  , atomic_min_idx_buffer
  , {
      uint data[];
    }
  );
#define get_atomic_min_idx()\
  GET_RESOURCE(atomic_min_idx_buffer,\
    g_generate_aabb_parameters[param_id].data.atomic_min_idx_buffer_id).data

REGISTER_BUFFER
  ( std430
  , writeonly
  , spheres_buffer
  , {
      SphereCull data[];
    }
  );
#define get_spheres()\
  GET_RESOURCE(spheres_buffer,\
    g_generate_aabb_parameters[param_id].data.atomic_min_idx_buffer_id).data
