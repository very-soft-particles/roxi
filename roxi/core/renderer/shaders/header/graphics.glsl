#include "common.glsl"
#include "graphics/vertex_data.glsl"
#include "graphics/camera_data.glsl"
#include "graphics/light_data.glsl"
#include "graphics/draw_indirect_command.glsl"
#include "graphics/clustered_forward_data.glsl"
#include "graphics/mesh_data.glsl"
#include "graphics/model_data.glsl"
#include "graphics/material_types.glsl"
#include "graphics/material_data.glsl"
#include "graphics/draw_params.glsl"
#include "graphics/frustum.glsl"

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
#define get_camera() \
  GET_RESOURCE(camera,   \
    GET_VARIABLE_NAME(draw_parameters)[param_id].data.camera_id).data

REGISTER_UNIFORM
  ( clustered_forward_data
  , {
      ClusteredForwardData data;
    }
  );
#define get_clustered_forward_data()  \
  GET_RESOURCE(clustered_forward_data,        \
    GET_VARIABLE_NAME(draw_parameters)[param_id].data.camera_id).data

REGISTER_BUFFER
  ( std430
  , readonly
  , draw_indirect_commands
  , {
      DrawIndirectCommand data[];
    }
  );
#define get_draw_commands()   \
  GET_RESOURCE(draw_indirect_commands,  \
      GET_VARIABLE_NAME(draw_parameters)[param_id].data.draw_command_buffer_id).data

REGISTER_BUFFER
  ( std430
  , readonly
  , draw_indexed_indirect_commands
  , {
      DrawIndexedIndirectCommand data[];
    }
  );
#define get_indexed_draw_commands()     \
  GET_RESOURCE(draw_indexed_indirect_commands,  \
    GET_VARIABLE_NAME(draw_parameters)[param_id].data.draw_command_buffer_id).data

REGISTER_BUFFER
  ( std430
  , readonly
  , materials
  , {
      MaterialData data[];
    }
  );
#define get_materials()     \
  GET_RESOURCE(materials,  \
    GET_VARIABLE_NAME(draw_parameters)[param_id].data.material_buffer_id).data

REGISTER_BUFFER
  ( std430
  , readonly
  , lights
  , {
      LightData data[];
    }
  );
#define get_lights()     \
  GET_RESOURCE(draw_indexed_indirect_commands,  \
    GET_VARIABLE_NAME(draw_parameters)[param_id].data.light_buffer_id).data


REGISTER_BUFFER
  ( std430
  , readonly
  , light_cells
  , {
      LightCell data[];
    }
  );
#define get_light_cells()     \
  GET_RESOURCE(light_cells,  \
    GET_VARIABLE_NAME(draw_parameters)[param_id].data.light_cell_buffer_id).data

REGISTER_BUFFER
  ( std430
  , readonly
  , light_indices
  , {
      uint data[];
    }
  );
#define get_light_indices()     \
  GET_RESOURCE(light_indices,  \
    GET_VARIABLE_NAME(draw_parameters)[param_id].data.light_index_buffer_id).data


REGISTER_BUFFER
  ( std430
  , readonly
  , models
  , {
      ModelData data[];
    }
  );
#define get_models()     \
  GET_RESOURCE(models,  \
    GET_VARIABLE_NAME(draw_parameters)[param_id].data.model_buffer_id).data

REGISTER_BUFFER
  ( std430
  , readonly
  , vertices
  , {
      VertexData data[];
    }
  );
#define get_vertices()     \
  GET_RESOURCE(vertices,  \
    GET_VARIABLE_NAME(draw_parameters)[param_id].data.vertex_buffer_id).data

REGISTER_BUFFER
  ( std430
  , readonly
  , indices
  , {
      uint data[];
    }
  );
#define get_indices()     \
  GET_RESOURCE(vertices,  \
    GET_VARIABLE_NAME(draw_parameters)[param_id].data.index_buffer_id).data

REGISTER_SPECIALIZATION_CONSTANT
  ( transparent
  , uint
  , 1     // ID = 0 is draw_params
  , 0     // by default not transparent
  );

/*
Notations
	V	View unit vector
	L	Incident light unit vector
	N	Surface normal unit vector
	H	Half unit vector between L and V
*/

#ifndef PI
#define PI 3.1415926535897932384626433832795
#endif

// Specular D
// Trowbridge-Reitz GGX that models the distribution of microfacet normal.
float distribution_ggx(float noh, float roughness)
{
	float alpha = roughness * roughness; // Disney remapping
	float alpha2 = alpha * alpha;
	float noh2 = noh * noh;

	float nominator = alpha2;
	float denominator = (noh2 * (alpha2 - 1.0) + 1.0);
	denominator = PI * denominator * denominator;

	return nominator / denominator;
}

// Roughness remapping for direct lighting (See Brian Karis's PBR Note)
float alpha_direct_lighting(float roughness)
{
	float r = (roughness + 1.0);
	float alpha = (r * r) / 8.0;
	return alpha;
}

// Specular G
// Geometry function that describes the self-shadowing property of the microfacets.
// When a surface is relatively rough, the surface's microfacets can overshadow other
// microfacets reducing the light the surface reflects.
float geometry_schlick_ggx(float nol, float nov, float alpha)
{
	float gl = nol / (nol * (1.0 - alpha) + alpha);
	float gv = nov / (nov * (1.0 - alpha) + alpha);
	return gl * gv;
}

// Specular F
// The Fresnel equation describes the ratio of surface reflection at different surface angles.
vec3 fresnel_schlick(float cos_theta, vec3 f0)
{
	return f0 + (1.0 - f0) * pow(clamp(1.0 - cos_theta, 0.0, 1.0), 5.0);
}

vec3 fresnel_schlick_roughness(float cos_theta, vec3 f0, float roughness)
{
	return f0 + (max(vec3(1.0 - roughness), f0) - f0) * pow(clamp(1.0 - cos_theta, 0.0, 1.0), 5.0);
}

// Basic Lambertian diffuse
vec3 diffuse(vec3 albedo)
{
	return albedo / PI;
}
