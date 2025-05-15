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

const vec3 vertex_p[3] = { vec3(0.3f, -0.3f, -0.1f), vec3(-0.3f, -0.3f, -0.1f), vec3(0.f, 0.3f, -0.1f) };

void main() {

 // if(gl_VertexIndex >= 3) {
 //   return;
 // }

  vec3 position = vertex_p[gl_VertexIndex];
  //vec3 position = GET_RESOURCE(vertex_buffer, 0).data[gl_VertexIndex].position;
  gl_Position = vec4(position.x, position.y, position.z, 1.f);
}
