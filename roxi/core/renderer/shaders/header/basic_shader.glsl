#include "common.glsl"
#include "graphics/vertex_data.glsl"
#include "graphics/camera_data.glsl"
#include "graphics/basic_draw_params.glsl"

REGISTER_UNIFORM
  ( draw_parameters
  , { 
      DrawParameters data;
    }
  );

REGISTER_UNIFORM
  ( camera
  , { 
      CameraData data;
    }
  );
#define get_camera(param_id) GET_RESOURCE(camera, GET_RESOURCE(draw_parameters, param_id).data.camera_id).data

REGISTER_BUFFER
  ( std430
  , readonly
  , vertex_data
  , {
      VertexData data;
    }
  );
#define get_vertices(param_id) GET_RESOURCE(vertices, GET_RESOURCE(draw_parameters, param_id).data.vertex_buffer_id).data

REGISTER_BUFFER
  ( std430
  , readonly
  , index_data
  , {
      uint data;
    }
  );
#define get_indices(param_id) GET_RESOURCE(indices, GET_RESOURCE(draw_parameters, param_id).data.index_buffer_id).data
