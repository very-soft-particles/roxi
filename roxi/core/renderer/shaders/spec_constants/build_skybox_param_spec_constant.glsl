REGISTER_SPECIALIZATION_CONSTANT
  ( skybox_param
  , uint
  , 0
  , 0
  );
#define get_skybox_param_id() const uint param_id =  GET_SPEC_CONSTANT(skybox_param)
