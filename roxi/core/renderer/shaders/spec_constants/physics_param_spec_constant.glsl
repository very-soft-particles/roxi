REGISTER_SPECIALIZATION_CONSTANT
  ( physics_param
  , uint
  , 3
  , 3
  );
#define get_physics_param_id() const uint param_id =  GET_SPEC_CONSTANT(physics_param)
