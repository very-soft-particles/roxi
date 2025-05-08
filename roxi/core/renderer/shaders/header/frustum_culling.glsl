#include "common.glsl"
#include "frustum_culling/frustum.glsl"
#include "frustum_culling/frustum_params.glsl"
#include "frustum_culling/model.glsl"
#include "graphics/mesh_data.glsl"
#include "graphics/draw_indirect_command.glsl"
#include "aabb.glsl"

REGISTER_UNIFORM
  ( frustum_parameters
  , {
      FrustumParameters data;
    }
  );

REGISTER_UNIFORM
  ( frustum
  , {
      Frustum data;
    }
  );
#define get_frustum()   \
  GET_RESOURCE(frustum,       \
    GET_VARIABLE_NAME(frustum_parameters)[param_id].data.frustum_buffer_id).data

REGISTER_BUFFER
  ( std430
  , readonly 
  , aabbs
  , {
      AABB data[];
    }
  );
#define get_aabbs()   \
  GET_RESOURCE(aabbs,       \
    GET_VARIABLE_NAME(frustum_parameters)[param_id].data.aabb_buffer_id).data

REGISTER_BUFFER
  ( std430
  , readonly
  , meshes
  , {
      MeshData data[];
    }
  );
#define get_meshes()     \
  GET_RESOURCE(meshes,  \
    GET_VARIABLE_NAME(frustum_parameters)[param_id].data.mesh_buffer_id).data

REGISTER_BUFFER
  ( std430
  , readonly
  , models
  , {
      ModelData data[];
    }
  );
#define get_models()   \
  GET_RESOURCE(models,       \
    GET_VARIABLE_NAME(frustum_parameters)[param_id].data.model_buffer_id).data

REGISTER_BUFFER
  ( std430
  , writeonly
  , draw_indexed_indirect_commands
  , {
      DrawIndexedIndirectCommand data[];
    }
  );
#define get_draw_indexed_indirect_commands()   \
  GET_RESOURCE(draw_indexed_indirect_commands,       \
    GET_VARIABLE_NAME(frustum_parameters)[param_id].data.draw_indirect_buffer_id).data

