#include "common.glsl"
#include "skinning/skinning_params.glsl"
#include "graphics/vertex_data.glsl"

REGISTER_UNIFORM
  ( skinning_parameters
  , {
      SkinningParameters data;
    }
  );

REGISTER_BUFFER
  ( std430
  , readonly
  , bone_matrices
  , {
      mat4 data[];
    }
  );
#define get_bones() \
    GET_RESOURCE(bone_matrices,\
      g_skinning_parameters[param_id].data.bone_buffer_id).data

REGISTER_BUFFER
  ( std430
  , readonly
  , bone_ids
  , {
      ivec4 data[];
    }
  );
#define get_bone_ids() \
    GET_RESOURCE(bone_ids,\
      g_skinning_parameters[param_id].data.bone_id_buffer_id).data

REGISTER_BUFFER
  ( std430
  , readonly
  , weights
  , {
      mat4 data[];
    }
  );
#define get_weights()\
    GET_RESOURCE(weights,\
      g_skinning_parameters[param_id].data.weight_buffer_id).data

REGISTER_BUFFER
  ( std430
  , readonly
  , skinning_indices
  , {
      mat4 data[];
    }
  );
#define get_skinning_indices()\
    GET_RESOURCE(skinning_indices,\
      g_skinning_parameters[param_id].data.skinning_index_buffer_id).data

REGISTER_BUFFER
  ( std430
  , readonly
  , vertices_in
  , {
      VertexData data[];
    }
  );
#define get_vertices_in(param_id)\
    GET_RESOURCE(vertices_in,\
      g_skinning_parameters[param_id].data.vert_in_buffer_id).data

REGISTER_BUFFER
  ( std430
  , writeonly
  , vertices_out
  , {
      VertexData data[];
    }
  );
#define get_vertices_out()\
    GET_RESOURCE(vertices_out,\
      g_skinning_parameters[param_id].data.vert_out_buffer_id).data

