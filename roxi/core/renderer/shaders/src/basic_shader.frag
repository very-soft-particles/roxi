#version 460 core

layout(location = 0) out vec4 frag_colour;


void main() {
  const vec3 COLOUR = vec3(0.2f, 0.4f, 0.0f);
  frag_colour = vec4(COLOUR, 1);
}
