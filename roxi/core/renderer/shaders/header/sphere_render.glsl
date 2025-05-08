#extension GL_ARB_shader_draw_parameters : enable

#include "common.glsl"
#include "sphere/sphere_cull_info.glsl"
#include "graphics/camera_data.glsl"


REGISTER_UNIFORM
  ( sphere_parameters
  , {
      SphereRenderParameters data;
    }
  );


REGISTER_UNIFORM
  ( camera
  , {
      CameraData data;
    }
  );
#define get_camera() \
  GET_RESOURCE(camera,   \
    g_sphere_parameters[param_id].data.camera_id).data

REGISTER_BUFFER
  ( std430
  , readonly
  , positions
  , {
      vec3 data[];
    }
  );
#define get_positions()  \
  GET_RESOURCE(positions,       \
    g_sphere_parameters[param_id].data.position_buffer_id).data\
  )

REGISTER_BUFFER
  ( std430
  , readonly
  , sizes
  , {
      float data[];
    }
  );

#define get_sizes()  \
  GET_RESOURCE(sizes,       \
    g_sphere_parameters[param_id].data.size_buffer_id).data\
  )

REGISTER_BUFFER
  ( std430
  , writeonly
  , cull_info 
  , {
      SphereCull data[];
    }
  );
#define get_cull_info_buffer()     \
    GET_RESOURCE(cull_info,               \
      g_sphere_parameters[param_id].data.cull_info_buffer_id).data \
  )

REGISTER_BUFFER
  ( std430
  , writeonly
  , spheres 
  , {
      Sphere data[];
    }
  );
#define get_sphere_buffer()     \
    GET_RESOURCE(spheres,               \
      g_sphere_parameters[param_id].data.sphere_buffer_id).data \
  )

