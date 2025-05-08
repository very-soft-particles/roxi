#version 460 core
#include "../header/common.glsl"

struct Vertex {
  vec3 position;
  uint _pad0;
};

REGISTER_SPECIALIZATION_CONSTANT
( vertex_buffer_id
, uint
, 0
, 0
);

REGISTER_UNIFORM
( vertex_buffer
, {
    Vertex data[3];
  }
);

void main() {
  const uint param_id = GET_SPEC_CONSTANT(vertex_buffer_id);

  if(gl_VertexIndex >= 3) {
    return;
  }

  vec3 position = GET_RESOURCE(vertex_buffer, param_id).data[gl_VertexIndex].position;
  gl_Position = vec4(position.x, position.y, position.z, 1.f);

}
