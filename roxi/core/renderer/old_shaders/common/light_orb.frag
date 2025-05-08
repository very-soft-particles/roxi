// from HelloVulkan
#version 460 core
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 frag_offset;
layout(location = 1) in vec4 circle_color;

layout(location = 0) out vec4 frag_color;

// maybe swap out for a uniform buffer containing the proper information for this light?
const float ORB_SIZE = 0.7;
const float GLOW_FALLOFF = 2.0;
const float GLOW_STRENGTH = 0.5;

void main()
{
	float dist = sqrt(dot(frag_offset, frag_offset));

	vec4 final_color = circle_color;

	if (dist >= ORB_SIZE)
	{
		final_color.a = 0;
	}

	float glow = pow(max(1.0 - pow(dist, GLOW_FALLOFF), 0.0), 2.0) * GLOW_STRENGTH;

	final_color += glow;

	if (dist >= 1.0)
	{
		final_color.a = 0;
	}

	frag_color = final_color;
}
