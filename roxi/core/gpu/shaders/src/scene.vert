#version 460 core
#include "header/common.glsl"

REGISTER_SPECIALIZATION_CONSTANT
  ( draw_param
  , uint
  , 0
  , 0
  );

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
#define get_camera(param_id) \
  GET_RESOURCE(camera,   \
    g_draw_parameters[param_id].data.camera_id).data

REGISTER_UNIFORM
  ( clustered_forward_data
  , {
      ClusteredForwardData data;
    }
  );
#define get_clustered_forward_data(param_id)  \
  GET_RESOURCE(clustered_forward_data,        \
    g_draw_parameters[param_id].data.camera_id).data

REGISTER_BUFFER
  ( std430
  , readonly
  , draw_indirect_commands
  , {
      DrawIndirectCommand data[];
    }
  );
#define get_draw_commands(param_id)   \
  GET_RESOURCE(draw_indirect_commands,  \
      g_draw_parameters[param_id].data.draw_command_buffer_id).data

REGISTER_BUFFER
  ( std430
  , readonly
  , draw_indexed_indirect_commands
  , {
      DrawIndexedIndirectCommand data[];
    }
  );
#define get_indexed_draw_commands(param_id)     \
  GET_RESOURCE(draw_indexed_indirect_commands,  \
    g_draw_parameters[param_id].data.draw_command_buffer_id).data

REGISTER_BUFFER
  ( std430
  , readonly
  , materials
  , {
      MaterialData data[];
    }
  );
#define get_materials(param_id)     \
  GET_RESOURCE(materials,  \
    g_draw_parameters[param_id].data.material_buffer_id).data

REGISTER_BUFFER
  ( std430
  , readonly
  , lights
  , {
      LightData data[];
    }
  );
#define get_lights(param_id)     \
  GET_RESOURCE(draw_indexed_indirect_commands,  \
    g_draw_parameters[param_id].data.light_buffer_id).data


REGISTER_BUFFER
  ( std430
  , readonly
  , light_cells
  , {
      LightCell data[];
    }
  );
#define get_light_cells(param_id)     \
  GET_RESOURCE(light_cells,  \
    g_draw_parameters[param_id].data.light_cell_buffer_id).data

REGISTER_BUFFER
  ( std430
  , readonly
  , light_indices
  , {
      uint data[];
    }
  );
#define get_light_indices(param_id)     \
  GET_RESOURCE(light_indices,  \
    g_draw_parameters[param_id].data.light_index_buffer_id).data


REGISTER_BUFFER
  ( std430
  , readonly
  , models
  , {
      ModelData data[];
    }
  );
#define get_models(param_id)     \
  GET_RESOURCE(models,  \
    g_draw_parameters[param_id].data.model_buffer_id).data

REGISTER_BUFFER
  ( std430
  , readonly
  , vertices
  , {
      VertexData data[];
    }
  );
#define get_vertices(param_id)     \
  GET_RESOURCE(vertices,  \
    g_draw_parameters[param_id].data.vertex_buffer_id).data

REGISTER_BUFFER
  ( std430
  , readonly
  , indices
  , {
      uint data[];
    }
  );
#define get_indices(param_id)     \
  GET_RESOURCE(vertices,  \
    g_draw_parameters[param_id].data.index_buffer_id).data

REGISTER_SPECIALIZATION_CONSTANT
  ( transparent
  , uint
  , 0     // by default not transparent
  , 1     // ID = 0 is draw_params
  );

layout(location = 0) out vec3 world_pos;
layout(location = 1) out vec2 tex_coord;
layout(location = 2) out vec3 normal;
layout(location = 3) out vec3 vertex_color;
layout(location = 4) out flat uint material_index;

layout(location = 0) in DrawIndexedIndirectCommand command;

void main() { 
  const uint param_id = GET_SPEC_CONSTANT(draw_param);

  if(command.instance_count == 0) {
    discard;
  }

  if(gl_VertexIndex > command.index_count) {
    discard;
  }

  ModelData model_data 
    = get_models(param_id)[command.first_instance];

  uint vert_offset = command.vertex_offset;
  uint index_offset = command.first_index;
  uint vert_index = get_indices(param_id)[index_offset + gl_VertexIndex] + vert_offset;

  VertexData vert_data = get_vertices(param_id)[vert_index];
  mat3 norm_matrix = transpose(inverse(mat3(model_data.model)));

  world_pos = (model_data.model * vec4(vert_data.position, 1.0)).xyz;
  tex_coord = vec2(vert_data.uv_x, vert_data.uv_y);
  normal = norm_matrix * vert_data.normal;
  vertex_color = vert_data.color.xyz;
  material_index = model.material_id;

  CameraData camera = get_camera(param_id);

  gl_Position = camera.projection * camera.view * vec4(world_pos, 1.0);
}
