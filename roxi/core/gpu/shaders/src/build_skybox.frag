#version 460 core

/*
Fragment shader for skybox
*/

layout(location = 0) in vec4 colour;

layout(location = 0) out vec4 frag_colour;

void main()
{
	frag_colour = colour;
};
