// to turn bindless off or on (taken verbatim from https://dev.to/gasim/implementing-bindless-design-in-vulkan-34no)
#extension GL_EXT_nonuniform_qualifier : enable
#define Bindless 1

#define PI 3.14159265358

#define UBOSet 0
#define SamplerSet 1
#define StorageImageSet 2
#define StorageBufferSet 3

// for easily accessing variables
#define GET_VARIABLE_NAME(Name) g_##Name##_register

#define GET_PUSH_CONSTANT_NAME(Name) k_##Name##_push_constant

#define GET_SPEC_CONSTANT_NAME(Name) k_##Name##_spec_constant

#define REGISTER_PUSH_CONSTANT(Name, Struct) \
  layout(push_constant) uniform Name##_t Struct GET_PUSH_CONSTANT_NAME(Name)

#define REGISTER_SPECIALIZATION_CONSTANT(Name, Type, ID, InitValue)  \
  layout(constant_id = ID) const Type GET_SPEC_CONSTANT_NAME(Name) = InitValue;

// registering variables
// uniform
#define REGISTER_UNIFORM(Name, Struct)  \
  layout(set = UBOSet, binding = 0)     \
    uniform Name##_t Struct                 \
    GET_VARIABLE_NAME(Name)[]

// storage
#define REGISTER_BUFFER(Layout, BufferAccess, Name, Struct)       \
  layout(Layout, set = StorageBufferSet, binding = 0)                     \
    BufferAccess buffer Name##_t Struct                               \
    GET_VARIABLE_NAME(Name)[]

// sampler
#define REGISTER_2D_SAMPLER(Name) \
  layout(set = SamplerSet, binding = 0) \
  uniform sampler2D GET_VARIABLE_NAME(Name)[]

#define REGISTER_3D_SAMPLER(Name) \
  layout(set = SamplerSet, binding = 0) \
  uniform sampler3D GET_VARIABLE_NAME(Name)[]

#define REGISTER_UNSIGNED_2D_SAMPLER(Name) \
  layout(set = SamplerSet, binding = 0) \
  uniform usampler2D GET_VARIABLE_NAME(Name)[]

#define REGISTER_UNSIGNED_3D_SAMPLER(Name) \
  layout(set = SamplerSet, binding = 0) \
  uniform usampler3D GET_VARIABLE_NAME(Name)[]

// storage image
#define REGISTER_2D_IMAGE(Name, Format) \
  layout(set = StorageImageSet, binding = 0, Format) \
  uniform image2D GET_VARIABLE_NAME(Name)[]

#define REGISTER_3D_IMAGE(Name, Format) \
  layout(set = StorageImageSet, binding = 0, Format) \
  uniform image3D GET_VARIABLE_NAME(Name)[]

#define GET_RESOURCE(Name, Index) \
  GET_VARIABLE_NAME(Name)[Index]

#define GET_PUSH_CONSTANT(Name) \
  GET_PUSH_CONSTANT_NAME(Name)

#define GET_SPEC_CONSTANT(Name) \
  GET_SPEC_CONSTANT_NAME(Name)

REGISTER_UNIFORM
  ( uniform_dummy
  , {
      uint ignore;
    }
  );

REGISTER_BUFFER
  ( std430
  , readonly
  , buffer_dummy
  , {
      uint ignore;
    }
  );

layout(set = SamplerSet, binding = 0) uniform sampler2D
g_global_textures_2D[];

layout(set = SamplerSet, binding = 0) uniform sampler3D
g_global_textures_3D[];

layout(set = StorageImageSet, binding = 0, rgba8_snorm) uniform image2D
g_global_images_2D[];

layout(set = StorageImageSet, binding = 0, rgba8_snorm) uniform image3D
g_global_images_3D[];
