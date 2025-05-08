#include "common.glsl"
#include "aabb/aabb.glsl"
#include "graphics/camera_data.glsl"

REGISTER_UNIFORM
  ( aabb_parameters
  , {
      AABBParameters data;
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
    g_aabb_parameters[param_id].data.camera_id).data

REGISTER_BUFFER
  ( std430
  , readonly
  , // TODO
