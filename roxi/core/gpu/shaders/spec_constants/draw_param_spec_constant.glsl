REGISTER_SPECIALIZATION_CONSTANT
  ( draw_param
  , uint
  , 0
  , 0
  );
#define get_draw_param_id() const uint param_id = GET_SPEC_CONSTANT(draw_param)
