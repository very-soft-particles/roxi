// =====================================================================================
//
//       Filename:  shader_config.cpp
//
//    Description:  
//
//        Version:  1.0
//        Created:  2024-08-05 9:23:06 AM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Roxi Graves (rg), nada
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#include <filesystem>
#include <vulkan/vulkan_core.h>
#include "vulkan/vulkan.h"
#include "spirv_reflect.h"
#include "../lofi/core/include/l_container.hpp"
#include "../lofi/core/include/l_arena.hpp"
#include "../lofi/core/include/l_file.hpp"

#define SPV_REFL_CHECK(result, error_msg) if((result) != SPV_REFLECT_RESULT_SUCCESS) { PRINT("%s\n", error_msg); return false; }

static constexpr u64 MaxDescriptorSets = 4;
static constexpr u64 MaxDescriptorSetBindings = 16;

struct VkDescriptorSetBindingMemberInfo {
  SpvReflectTypeFlags flags{};
  char name[64];
  u32 num_dimensions = MAX_u32;
  lofi::mem::ArrayContainerPolicy<VkDescriptorSetBindingMemberInfo*, 32, 8, lofi::mem::StackAllocPolicy> members{};
};

struct VkDescriptorSetBindingInfo {
  SpvReflectDescriptorType type;
  // null if type is not image
  SpvReflectFormat format;
  // image_dimension = 0 if type is not image
  u32 image_dimension = 0;
  lofi::mem::ArrayContainerPolicy<VkDescriptorSetBindingMemberInfo, 32, 8, lofi::mem::StackAllocPolicy> members{};
};

struct VkDescriptorSetLayoutInfo {
  u32 set = MAX_u32;
  u32 binding_count = MAX_u32;
  lofi::mem::ArrayContainerPolicy<VkDescriptorSetBindingInfo, MaxDescriptorSetBindings, 8, lofi::mem::StackAllocPolicy> binding_infos;
  lofi::mem::ArrayContainerPolicy<char[64], MaxDescriptorSetBindings, 8, lofi::mem::StackAllocPolicy> binding_names;
};

struct VkPipelineLayoutInfo {
  char shader_name[64];
  u32 stage = MAX_u32;
  u32 set_count = MAX_u32;
  // max 4 descriptor sets
  lofi::mem::ArrayContainerPolicy<VkDescriptorSetLayoutInfo, MaxDescriptorSets, 8, lofi::mem::SubAllocPolicy> sets;
};

struct VkVertexInputInfo {
  u32 shader_descriptor_id = MAX_u32;
  u32 location = MAX_u32;
  char name[64];
  u32 binding = MAX_u32;
  SpvReflectFormat format = SPV_REFLECT_FORMAT_UNDEFINED;
  u32 offset = MAX_u32;
};

struct VkFragmentInputInfo {
  u32 shader_descriptor_id = MAX_u32;
  u32 location = MAX_u32;
  char name[64];
  u32 binding = MAX_u32;
  SpvReflectFormat format = SPV_REFLECT_FORMAT_UNDEFINED;
  u32 offset = MAX_u32;
};

struct VkVertexOutputInfo {
  u32 shader_descriptor_id = MAX_u32;
  u32 location = MAX_u32;
  char name[64];
  u32 binding = MAX_u32;
  SpvReflectFormat format = SPV_REFLECT_FORMAT_UNDEFINED;
  u32 offset = MAX_u32;
};

struct VkFragmentOutputInfo {
  u32 shader_descriptor_id = MAX_u32;
  u32 location = MAX_u32;
  char name[64];
  u32 binding = MAX_u32;
  SpvReflectFormat format = SPV_REFLECT_FORMAT_UNDEFINED;
  u32 offset = MAX_u32;
};

struct ShaderInfo {
  SpvReflectShaderModule mod;
  lofi::String shader_name;
  char string_buffer[256];
};


//
//lofi::mem::ArrayContainerPolicy<VkPipelineLayoutInfo, 256, 8, lofi::mem::MAllocPolicy> shader_infos;
//lofi::mem::ArrayContainerPolicy<VkVertexInputInfo, 256, 8, lofi::mem::MAllocPolicy> vertex_input_infos;
//lofi::mem::ArrayContainerPolicy<VkVertexOutputInfo, 256, 8, lofi::mem::MAllocPolicy> vertex_output_infos;
//lofi::mem::ArrayContainerPolicy<VkFragmentInputInfo, 256, 8, lofi::mem::MAllocPolicy> fragment_input_infos;
//lofi::mem::ArrayContainerPolicy<VkFragmentOutputInfo, 256, 8, lofi::mem::MAllocPolicy> fragment_output_infos;



lofi::mem::ArrayContainerPolicy<ShaderInfo, 256, 8, lofi::mem::MAllocPolicy> shader_modules;
lofi::mem::ArrayContainerPolicy
  < lofi::mem::ArrayContainerPolicy<SpvReflectDescriptorSet*, 8, 8, lofi::mem::StackAllocPolicy>
  , 256, 8, lofi::mem::MAllocPolicy> sets;
lofi::mem::ArrayContainerPolicy
  < lofi::mem::ArrayContainerPolicy<SpvReflectInterfaceVariable*, 8, 8, lofi::mem::StackAllocPolicy>
  , 256, 8, lofi::mem::MAllocPolicy> inputs;
lofi::mem::ArrayContainerPolicy
  < lofi::mem::ArrayContainerPolicy<SpvReflectInterfaceVariable*, 8, 8, lofi::mem::StackAllocPolicy>
  , 256, 8, lofi::mem::MAllocPolicy> outputs;

static const lofi::String get_vk_descriptor_binding_type_string(SpvReflectDescriptorType descriptor_type) {
  if(descriptor_type == SPV_REFLECT_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR) {
    return lofi::str_cstring("VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR");
  }
  if(descriptor_type == SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLED_IMAGE) {
    return lofi::str_cstring("VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE");
  }
  if(descriptor_type == SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLER) {
    return lofi::str_cstring("VK_DESCRIPTOR_TYPE_SAMPLER");
  }
  if(descriptor_type == SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {
    return lofi::str_cstring("VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER");
  }
  if(descriptor_type == SPV_REFLECT_DESCRIPTOR_TYPE_INPUT_ATTACHMENT) {
    return lofi::str_cstring("VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT");
  }
  if(descriptor_type == SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER) {
    return lofi::str_cstring("VK_DESCRIPTOR_TYPE_STORAGE_BUFFER");
  }
  if(descriptor_type == SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC) {
    return lofi::str_cstring("VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC");
  }
  if(descriptor_type == SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_IMAGE) {
    return lofi::str_cstring("VK_DESCRIPTOR_TYPE_STORAGE_IMAGE");
  }
  if(descriptor_type == SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER) {
    return lofi::str_cstring("VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER");
  }
  if(descriptor_type == SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
    return lofi::str_cstring("VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER");
  }
  if(descriptor_type == SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC) {
    return lofi::str_cstring("VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC");
  }
  if(descriptor_type == SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER) {
    return lofi::str_cstring("VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER");
  }
  else {
    return lofi::str_cstring("ROXI_TYPE_NOT_SUPPORTED");
  }

}
static const lofi::String get_vk_pipeline_stage_flags_string(auto* arena, SpvReflectShaderStageFlagBits spv_refl_flags) {
  lofi::StringList list{};
  if(spv_refl_flags & SPV_REFLECT_SHADER_STAGE_VERTEX_BIT) {
    lofi::str_list_push(arena, &list, lofi::str_cstring("VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT"));
  }
  if(spv_refl_flags & SPV_REFLECT_SHADER_STAGE_TESSELLATION_CONTROL_BIT    ){
    lofi::str_list_push(arena, &list, lofi::str_cstring("VK_PIPELINE_STAGE_2_TESSELLATION_CONTROL_SHADER_BIT"));
  } 
  if(spv_refl_flags & SPV_REFLECT_SHADER_STAGE_TESSELLATION_EVALUATION_BIT ){
    lofi::str_list_push(arena, &list, lofi::str_cstring("VK_PIPELINE_STAGE_2_TESSELLATION_EVALUATION_SHADER_BIT"));
  } 
  if(spv_refl_flags & SPV_REFLECT_SHADER_STAGE_GEOMETRY_BIT                ){
    lofi::str_list_push(arena, &list, lofi::str_cstring("VK_PIPELINE_STAGE_2_GEOMETRY_SHADER_BIT"));
  } 
  if(spv_refl_flags & SPV_REFLECT_SHADER_STAGE_FRAGMENT_BIT                ){
    lofi::str_list_push(arena, &list, lofi::str_cstring("VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT"));
  } 
  if(spv_refl_flags & SPV_REFLECT_SHADER_STAGE_COMPUTE_BIT                 ){
    lofi::str_list_push(arena, &list, lofi::str_cstring("VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT"));
  } 
  if(spv_refl_flags & SPV_REFLECT_SHADER_STAGE_TASK_BIT_NV                 ){
    lofi::str_list_push(arena, &list, lofi::str_cstring("VK_PIPELINE_STAGE_2_TASK_SHADER_BIT_NV"));
  } 
  if(spv_refl_flags & SPV_REFLECT_SHADER_STAGE_TASK_BIT_EXT                ){
    lofi::str_list_push(arena, &list, lofi::str_cstring("VK_PIPELINE_STAGE_2_TASK_SHADER_BIT_EXT"));
  }
  if(spv_refl_flags & SPV_REFLECT_SHADER_STAGE_MESH_BIT_NV                 ){
    lofi::str_list_push(arena, &list, lofi::str_cstring("VK_PIPELINE_STAGE_2_MESH_SHADER_BIT_NV"));
  } 
  if(spv_refl_flags & SPV_REFLECT_SHADER_STAGE_MESH_BIT_EXT                ){
    lofi::str_list_push(arena, &list, lofi::str_cstring("VK_PIPELINE_STAGE_2_MESH_SHADER_BIT_EXT"));
  }
  lofi::StringJoin joiner{};
  joiner.mid = lofi::str_cstring(" | ");
  lofi::String result = lofi::str_join(arena, &list, &joiner);

  return result;



}
static VkPipelineStageFlags get_vk_pipeline_stage_flags(SpvReflectShaderStageFlagBits spv_refl_flags) {
  VkPipelineStageFlags2 result = 0;
  if(spv_refl_flags & SPV_REFLECT_SHADER_STAGE_VERTEX_BIT) {
    result |= VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT;
  }
  if(spv_refl_flags & SPV_REFLECT_SHADER_STAGE_TESSELLATION_CONTROL_BIT    ){
    result |= VK_PIPELINE_STAGE_2_TESSELLATION_CONTROL_SHADER_BIT;
  } 
  if(spv_refl_flags & SPV_REFLECT_SHADER_STAGE_TESSELLATION_EVALUATION_BIT ){
    result |= VK_PIPELINE_STAGE_2_TESSELLATION_EVALUATION_SHADER_BIT;
  } 
  if(spv_refl_flags & SPV_REFLECT_SHADER_STAGE_GEOMETRY_BIT                ){
    result |= VK_PIPELINE_STAGE_2_GEOMETRY_SHADER_BIT;
  } 
  if(spv_refl_flags & SPV_REFLECT_SHADER_STAGE_FRAGMENT_BIT                ){
    result |= VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
  } 
  if(spv_refl_flags & SPV_REFLECT_SHADER_STAGE_COMPUTE_BIT                 ){
    result |= VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
  } 
  if(spv_refl_flags & SPV_REFLECT_SHADER_STAGE_TASK_BIT_NV                 ){
    result |= VK_PIPELINE_STAGE_2_TASK_SHADER_BIT_NV;
  } 
  if(spv_refl_flags & SPV_REFLECT_SHADER_STAGE_TASK_BIT_EXT                ){
    result |= VK_PIPELINE_STAGE_2_TASK_SHADER_BIT_EXT;
  }
  if(spv_refl_flags & SPV_REFLECT_SHADER_STAGE_MESH_BIT_NV                 ){
    result |= VK_PIPELINE_STAGE_2_MESH_SHADER_BIT_NV;
  } 
  if(spv_refl_flags & SPV_REFLECT_SHADER_STAGE_MESH_BIT_EXT                ){
    result |= VK_PIPELINE_STAGE_2_MESH_SHADER_BIT_EXT;
  }
  return result;
}
  // Garbage or recycle??
 // const u32 shader_descriptor_id = shader_infos.get_size();
 // VkPipelineLayoutInfo* shader_descriptor = shader_infos.push(1);
 // lofi::String shader_out_name = lofi::str((u8*)shader_descriptor->shader_name, 0);
 // lofi::str_copy_nt(shader_name, &shader_out_name, 64);
 // shader_descriptor->stage = modshader_stage;
 // shader_descriptor->set_count = count;
 // for(u64 i = 0; i < count; i++) {
 //   const u64 binding_count = sets[i]->binding_count;
 //   shader_descriptor->sets[i].set = sets[i]->set;
 //   shader_descriptor->sets[i].binding_count = binding_count;
 //   for(u64 j = 0; j < binding_count; j++) {
 //     // get descriptor_type
 //     SpvReflectDescriptorType descriptor_type = sets[i]->bindings[j]->descriptor_type;
 //     shader_descriptor->sets[i].binding_infos[j].type = descriptor_type;

 //     // get binding_name
 //     lofi::String binding_name = lofi::nt_str_cstring(sets[i]->bindings[j]->name);
 //     lofi::str_copy_nt(binding_name, ((char*)shader_descriptor->sets[i].binding_names[j]), 64);

 //     // get binding type information
 //     SpvReflectTypeFlags type = sets[i]->bindings[j]->type_description->type_flags;

 //     // add member informations, first index is always the information for the
 //     // binding, subsequent members are the individual bindings for nested structures
 //     const u64 member_count = sets[i]->bindings[j]->type_description->member_count;
 //     VkDescriptorSetBindingMemberInfo* info = shader_descriptor->sets[i].binding_infos[j].members.push(member_count);
 //     info->flags = type;
 //     for(u64 k = 0; k < member_count; k++) {
 //       const SpvReflectTypeDescription& current_member = sets[i]->bindings[j]->type_description->members[k];
 //       VkDescriptorSetBindingMemberInfo* new_root_member = &shader_descriptor->sets[i].binding_infos[j].members[k];
 //       SpvReflectTypeFlags current_type = current_member.type_flags;
 //       new_root_member->flags = current_type;

 //       lofi::String member_name = lofi::nt_str_cstring(current_member.type_name);
 //       lofi::str_copy_nt(member_name, (char*)&new_root_member->name, 64);

 //       auto handle_type = [&](SpvReflectTypeFlags type, VkDescriptorSetBindingInfo& out_members, const SpvReflectTypeDescription* const type_description) {

 //         if(type & SPV_REFLECT_TYPE_FLAG_STRUCT) {
 //           const u64 struct_member_count = type_description->struct_type_description->member_count;
 //           VkDescriptorSetBindingMemberInfo* begin = out_members.members.push(struct_member_count);
 //           VkDescriptorSetBindingMemberInfo* root = begin - 1;
 //           VkDescriptorSetBindingMemberInfo** ptr_begin = root->members.push(struct_member_count);
 //           for(u64 l = 0; l < struct_member_count; l++) {
 //             // get current struct member description
 //             const SpvReflectTypeDescription* const current_description = type_description->members + l;

 //             // get current struct member info
 //             VkDescriptorSetBindingMemberInfo* current_struct_member_info = begin + l;
 //             *(ptr_begin + l) = current_struct_member_info;
 //             current_struct_member_info->flags = current_description->type_flags;

 //             // get struct member name
 //             lofi::String struct_member_name = lofi::nt_str_cstring(current_description->struct_member_name);
 //             lofi::str_copy_nt(struct_member_name, current_struct_member_info->name, 64);
 //           }
 //         }
 //         if(type & SPV_REFLECT_TYPE_FLAG_ARRAY) {
 //           VkDescriptorSetBindingMemberInfo* member_ptr = out_members.members.push(1);



 //         }
 //         if(type & SPV_REFLECT_TYPE_FLAG_BOOL) {

 //         }
 //         if(type & SPV_REFLECT_TYPE_FLAG_FLOAT) {

 //         }
 //         if(type & SPV_REFLECT_TYPE_FLAG_INT) {

 //         }
 //         if(type & SPV_REFLECT_TYPE_FLAG_VECTOR) {

 //         }
 //         if(type & SPV_REFLECT_TYPE_FLAG_MATRIX) {

 //         }
 //         if(type & SPV_REFLECT_TYPE_FLAG_EXTERNAL_ACCELERATION_STRUCTURE) {

 //         }
 //         if(type & SPV_REFLECT_TYPE_FLAG_EXTERNAL_BLOCK) {

 //         }
 //         if(type & SPV_REFLECT_TYPE_FLAG_EXTERNAL_IMAGE) {

 //         }
 //         if(type & SPV_REFLECT_TYPE_FLAG_EXTERNAL_MASK) {

 //         }
 //         if(type & SPV_REFLECT_TYPE_FLAG_EXTERNAL_SAMPLED_IMAGE) {

 //         }
 //         if(type & SPV_REFLECT_TYPE_FLAG_EXTERNAL_SAMPLER) {

 //         }
 //         if(type & SPV_REFLECT_TYPE_FLAG_REF) {

 //         }
 //         if(type & SPV_REFLECT_TYPE_FLAG_VOID) {

 //         }
 //         if(type & SPV_REFLECT_TYPE_FLAG_UNDEFINED) {

 //         }
 //       };
 //         // get number of struct members
 //       handle_type(current_type, shader_descriptor->sets[i].binding_infos[j], (SpvReflectTypeDescription*)sets[i]->bindings[j]->type_description);
 // 
 //         // allocate sub-member pointers
 //         VkDescriptorSetBindingMemberInfo** struct_members = new_root_member->members.push(struct_member_count);
 //         // allocate sub-member informations
 //         VkDescriptorSetBindingMemberInfo* begin = shader_descriptor->sets[i].binding_infos[j].members.push(struct_member_count);
 //         while(current_index) {
 //           // set the sub member pointer
 //           VkDescriptorSetBindingMemberInfo* current_member = begin + current_index;
 //           *(out_ptr + current_index) = current_member;

 //           // get member_name
 //           lofi::String member_name = lofi::nt_str_cstring(sets[i]->bindings[j]->type_description->members[current_index].struct_member_name);
 //           lofi::str_copy_nt(member_name, current_member->name, 64);
 //           current_index--;
 //         }
 //           // get member_flags
 //         SpvReflectTypeFlags member_flags = sets[i]->bindings[j]->type_description->struct_type_description->members[member_count - 1].type_flags;
 //         current_type = member_flags;

 //         current_member_count += member_count;
 //       }
 //       current_member_count--;

 //     }

 //   } 


b8 parse_spirv_file(const char* file_path) {
 
  lofi::File file = lofi::File::create<lofi::FileType::ReadOnly>(file_path);

  if(!file.is_open()) {
    PRINT_LINE("[FATAL] failed to open file");
    return false;
  }

  const u64 spirv_size = file.get_size();

  void* buffer = malloc(spirv_size);
  if(buffer == nullptr) {
    PRINT_LINE("[FATAL] failed to allocate buffer for spirv data");
    return false;
  }

  if(!file.copy_all_to_buffer(buffer)) {
    PRINT_LINE("[FATAL] failed to copy spirv data to buffer");
    return false;
  }

  if(!file.close()) {
    PRINT_LINE("[FATAL] failed to close spirv file");
    return false;
  }

  lofi::String shader_path = lofi::str_cstring(file_path);
  const u64 period_offset = lofi::str_find_next_last('.', 2, shader_path);
#if (OS_WINDOWS)
  const u32 slash_offset = lofi::str_find_last('\\', shader_path);
#elif (OS_MAC || OS_LINUX)
  const u32 slash_Offset = lofi::str_find_last('/', shader_path);
#endif
  shader_path = lofi::str8_prefix(shader_path, period_offset);
  shader_path = lofi::str8_postfix(shader_path, slash_offset);

  ShaderInfo* info = shader_modules.push(1);
  info->shader_name.str = (u8*)info->string_buffer;
  info->shader_name.size = 0;
  lofi::str_copy(shader_path, &info->shader_name, 256);
  auto* sets_begin = sets.push(1);
  auto* inputs_begin = inputs.push(1);
  auto* outputs_begin = outputs.push(1);

  SPV_REFL_CHECK(spvReflectCreateShaderModule2(SPV_REFLECT_MODULE_FLAG_NO_COPY, spirv_size, buffer, &info->mod)
      , "failed to create shader module");
  
  u32 count = 0;

  SPV_REFL_CHECK(spvReflectEnumerateDescriptorSets(&info->mod, &count, nullptr)
      , "failed to count descriptor sets");
  SPV_REFL_CHECK(spvReflectEnumerateDescriptorSets(&info->mod, &count, sets_begin->push(count))
      , "failed to enumerate descriptor sets");
  if(info->mod.shader_stage == SPV_REFLECT_SHADER_STAGE_VERTEX_BIT) {
    count = 0;
    SPV_REFL_CHECK(spvReflectEnumerateInputVariables(&info->mod, &count, nullptr)
      , "failed to count input variables");
    SPV_REFL_CHECK(spvReflectEnumerateInputVariables(&info->mod, &count, inputs_begin->push(count))
      , "failed to enumerate input variables");

    count = 0;
    SPV_REFL_CHECK(spvReflectEnumerateOutputVariables(&info->mod, &count, nullptr)
      , "failed to count output variables");
    SPV_REFL_CHECK(spvReflectEnumerateOutputVariables(&info->mod, &count, outputs_begin->push(count))
      , "failed to enumerate output variables");

  } else if(info->mod.shader_stage == SPV_REFLECT_SHADER_STAGE_FRAGMENT_BIT) {
    count = 0;
    SPV_REFL_CHECK(spvReflectEnumerateInputVariables(&info->mod, &count, nullptr)
      , "failed to count input variables");
    SPV_REFL_CHECK(spvReflectEnumerateInputVariables(&info->mod, &count, inputs_begin->push(count))
      , "failed to enumerate input variables");

    count = 0;
    SPV_REFL_CHECK(spvReflectEnumerateOutputVariables(&info->mod, &count, nullptr)
      , "failed to count output variables");
    SPV_REFL_CHECK(spvReflectEnumerateOutputVariables(&info->mod, &count, outputs_begin->push(count))
      , "failed to enumerate output variables");
  }
  free(buffer);
  return true;
}

b8 iterate_spirv_directory_and_parse(const char* spirv_directory) {
  using iter_t = std::filesystem::directory_iterator;

  PRINT("spirv directory path in reflect function: %s\n", spirv_directory);

  iter_t iter{spirv_directory};

  for(auto& file_path : iter) {
    PRINT("spirv file found at: %s\n", file_path.path().string().c_str());
    if(!parse_spirv_file(file_path.path().string().c_str())) {
      PRINT("[ERROR] failed to parse file %s\n", file_path.path().string().c_str());
      return false;
    }
  }

  PRINT("spirv reflect finished parsing: %s\n",spirv_directory);
  
  return true;
}

b8 write_cpp_file(const char* file_path) {

  PRINT("writing cpp file at %s\n", file_path);

  lofi::File file = lofi::File::create<lofi::FileType::WriteOnly>(file_path);

  if(!file.is_open()) {
    PRINT("[FATAL] failed to open cpp output file %s\n", file_path);
    return false;
  }


  const u64 num_shaders = shader_modules.get_size();
  PRINT("total num shaders = %llu\n", num_shaders);
  file << "#pragma once" << lofi::NewLine
    << "#include \"rx_vocab.h\"" 

    << lofi::NewLine
    << "namespace roxi {" 

    << lofi::NewLine << lofi::Tab 
    << "namespace config {" 

    << lofi::NewLine << lofi::Tab << lofi::Tab 
    << "namespace shaders {" 

    << lofi::NewLine << lofi::Tab << lofi::Tab << lofi::Tab 
    << "inline constexpr u64 TotalNumShaders = " << num_shaders << ';'

    << lofi::NewLine << lofi::Tab << lofi::Tab << lofi::Tab
    << "inline constexpr const char* shader_names[] = { " ;
  PRINT_LINE("getting shader names");
  for(u64 i = 0; i < num_shaders; i++) {
    PRINT("shader name %llu\n", i);
    if(i != 0) {
      file << ", ";
      if(!(i & 3)) {
        file << lofi::NewLine << lofi::Tab << lofi::Tab << lofi::Tab << lofi::Tab;
      }
    }
    for(u64 j = 0; j < shader_modules[i].shader_name.size; j++) {
      PRINT("%c", shader_modules[i].shader_name.str[j]);
    }
    PRINT_S("\n");
    file << "\"" 
      << shader_modules[i].shader_name
      << "\"";
  }
  file << lofi::NewLine
    << lofi::NewLine << lofi::Tab << lofi::Tab << lofi::Tab
    << "};" << lofi::NewLine
    << lofi::NewLine << lofi::Tab << lofi::Tab << lofi::Tab

    << "inline constexpr u64 shader_descriptor_set_counts[] = {" << lofi::NewLine;
  PRINT_LINE("getting shader descriptor set counts");
  for(u64 i = 0; i < num_shaders; i++) {
    if(i != 0) {
      file << ", ";
      if(!(i & 3)) {
        file << lofi::NewLine << lofi::Tab << lofi::Tab << lofi::Tab << lofi::Tab;
      }
    }
    file << shader_modules[i].mod.descriptor_set_count;
  }

  file << lofi::NewLine << lofi::Tab << lofi::Tab << lofi::Tab
    << "};" << lofi::NewLine
    << lofi::NewLine << lofi::Tab << lofi::Tab << lofi::Tab

    << "inline constexpr VkPipelineStageFlags2 shader_stages[] = {" << lofi::NewLine;
  for(u64 i = 0; i < num_shaders; i++) {
    if(i != 0) {
      file << ", ";
      if(!(i & 3)) {
        file << lofi::NewLine << lofi::Tab << lofi::Tab << lofi::Tab << lofi::Tab;
      }
    }
    lofi::Arena<KB(4), 8, lofi::mem::StackAllocPolicy> arena;
    file << "( " 
      << get_vk_pipeline_stage_flags_string(&arena, shader_modules[i].mod.shader_stage) 
      << " )";
  }

    file << lofi::NewLine << lofi::Tab << lofi::Tab << lofi::Tab
    << "};"
    << lofi::NewLine << lofi::Tab << lofi::Tab << lofi::Tab

    << "inline constexpr u32 shader_descriptor_set_binding_counts[] = {" << lofi::NewLine;
  for(u64 i = 0; i < num_shaders; i++) {
    const u64 descriptor_set_count = shader_modules[i].mod.descriptor_set_count; 
    for(u64 j = 0; j < descriptor_set_count; j++) {
      if(i != 0 || j != 0) {
        file << ", ";
        if(!(j & 7)) {
          file << lofi::NewLine << lofi::Tab << lofi::Tab << lofi::Tab << lofi::Tab;
        }
      }
      file << shader_modules[i].mod.descriptor_sets[j].binding_count;
    }
  }

  file << lofi::NewLine << lofi::Tab << lofi::Tab << lofi::Tab
  << "};"
  << lofi::NewLine << lofi::Tab << lofi::Tab << lofi::Tab


  //<< "inline constexpr const char* shader_descriptor_set_binding_names[] = {" << lofi::NewLine;
  //for(u64 i = 0; i < num_shaders; i++) {
  //  const u64 descriptor_set_count = shader_modules[i].mod.descriptor_set_count; 
  //  for(u64 j = 0; j < descriptor_set_count; j++) {
  //    const u64 descriptor_set_binding_count = shader_modules[i].mod.descriptor_sets[j].binding_count;
  //    for(u64 k = 0; k < descriptor_set_binding_count; k++) {
  //      if(k != 0) {
  //        file << ", ";
  //        if(!(k & 3)) {
  //          file << lofi::NewLine << lofi::Tab << lofi::Tab << lofi::Tab << lofi::Tab;
  //        }
  //      }
  //      file << "\""
  //        << shader_modules[i].mod.descriptor_sets[j].bindings[k]->name
  //        << "\"";
  //    }
  //    file << " }";
  //  }
  //  file << " }";
  //}

  //file << lofi::NewLine << lofi::Tab << lofi::Tab << lofi::Tab
  //<< "};"
  //<< lofi::NewLine << lofi::Tab << lofi::Tab << lofi::Tab


  << "inline constexpr VkDescriptorType shader_descriptor_set_binding_types[] = {" << lofi::NewLine;
  for(u64 i = 0; i < num_shaders; i++) {
    const u64 descriptor_set_count = shader_modules[i].mod.descriptor_set_count; 
    for(u64 j = 0; j < descriptor_set_count; j++) {
      const u64 descriptor_set_binding_count = shader_modules[i].mod.descriptor_sets[j].binding_count;
      for(u64 k = 0; k < descriptor_set_binding_count; k++) {
        if(i != 0 || j != 0 || k != 0) {
          file << ", ";
          if(!(k & 3)) {
            file << lofi::NewLine << lofi::Tab << lofi::Tab << lofi::Tab << lofi::Tab;
          }
        }
        file << get_vk_descriptor_binding_type_string(shader_modules[i].mod.descriptor_sets[j].bindings[k]->descriptor_type);
      }
    }
  }
  file << lofi::NewLine << lofi::Tab << lofi::Tab << lofi::Tab 
  << "};" << lofi::NewLine
  << lofi::NewLine << lofi::Tab << lofi::Tab << 
  "}		// -----  end of namespace shaders  ----- " 
  << lofi::NewLine << lofi::Tab << 
  "} 	 // -----  end of namespace config  ----- " 
  << lofi::NewLine 
  << "}	  // -----  end of namespace roxi  ----- "
  << lofi::NewLine;

  file.close();

  return true;
}

int main(int argc, char** argv) {

  PRINT_LINE("entering spirv reflect");
  if(argc != 3) {
    PRINT("[ERROR] Incorrect number of arguments used. Expected 2, found %i.\nformat should be 1: spirv in directory, 2: shader resources file path\n", argc - 1);
    return 1;
  }

  if(shader_modules.get_buffer() == nullptr) {
    PRINT_LINE("[ERROR] could not allocate heap memory for descriptor infos");
    return 1;
  }
 
  PRINT("iterating directory %s\n", argv[1]);
  if(!iterate_spirv_directory_and_parse(argv[1])) {
    PRINT("[ERROR] could not parse spirv directory: %s\n", argv[1]);
    return 1;
  }
  PRINT("writing file %s\n", argv[2]);
  if(!write_cpp_file(argv[2])) {
    PRINT("[ERROR] failed to write cpp file %s\n", argv[2]);
    return 1;
  }

  PRINT_LINE("spirv successfully reflected, exiting shader_config");
  return 0;
}
