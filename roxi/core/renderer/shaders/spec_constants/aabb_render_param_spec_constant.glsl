REGISTER_SPECIALIZATION_CONSTANT
  ( aabb_render_param
  , uint
  , 2
  , 2
  );
#define get_aabb_render_param_id() const uint param_id =  GET_SPEC_CONSTANT(aabb_render_param)
