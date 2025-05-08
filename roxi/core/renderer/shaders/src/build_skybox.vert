// from HelloVulkan
#version 460 core

#include "../header/build_skybox_vert.glsl"
#include "../spec_constants/build_skybox_param_spec_constant.glsl"

/*
Vertex shader to generate a skybox
*/

layout(location = 0) out vec4 colour;

void main()
{
  const uint param_id = GET_SPEC_CONSTANT(skybox_param);
	int idx = CUBE_INDICES[gl_VertexIndex];
	vec4 pos4 = vec4(CUBE_POS[idx], 1.0);

  CameraData cam_ubo = get_camera(param_id);

	// depthCompareOp should be VK_COMPARE_OP_LESS_OR_EQUAL
	gl_Position = vec4(CUBE_POS[idx], 1);
  colour = get_colour(param_id).colour_at_pos[idx].xyzw;
	
}
