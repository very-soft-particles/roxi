#include "common.glsl"
#include "cube/cube.glsl"
#include "graphics/camera_data.glsl"

REGISTER_UNIFORM
  ( skybox_parameters
  , {
      SkyboxParameters data;
    }
  );

REGISTER_UNIFORM
  ( colour
  , {
      ColourData data;
    }
  );
#define get_colour() \
    GET_RESOURCE(colour,      \
      GET_VARIABLE_NAME(skybox_parameters)[param_id].data.colour_id).data

