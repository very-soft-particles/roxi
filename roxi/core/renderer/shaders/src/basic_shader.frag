#version 460 core

layout(location = 0) out vec4 frag_colour;

const vec3 COLOUR = vec3(1.f, 0.f, 0.f);

void main() {
  frag_colour = vec4(COLOUR, 1.f);
}
