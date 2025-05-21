// =====================================================================================
//
//       Filename:  rx_resource_manager.hpp
//
//    Description:  a helper for caching and searching for resources 
//
//        Version:  1.0
//        Created:  2024-07-24 10:08:09 AM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Roxi Graves (rg), nada
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#pragma once
#include "rx_log.hpp"
#include "rx_file.hpp"
#include "mesh_resources.hpp"
#include "audio_resources.hpp"
#include "ecs_resources.hpp"
#include "shader_resources.hpp"
#include <vulkan/vulkan_core.h>


namespace roxi {
  struct Extent {
    u32 width;
    u32 height;
    u32 depth;
  };

  enum class PixelType {
    iR8G8B8A8,    // Signed Integer
    uR8G8B8A8,    // Unsigned Integer
    nR8G8B8A8,    // Normed
    sR8G8B8A8,    // Scaled
    Max
  };

  namespace resource {

    namespace helpers {
     
      static u64 find_shader_index_from_shader_name(const char* shader_name) {
        const u64 shader_count = config::shaders::TotalNumShaders;
        String search_name = shader_name;
        search_name.size -= 2;
        for(u64 i = 0; i < shader_count; i++) {
          String current_shader_name = config::shaders::shader_names[i];
          const u32 start_idx = lofi::str_find_last('/', current_shader_name);
          const u32 final_idx = lofi::str_find_last('.', current_shader_name);
          current_shader_name.str = current_shader_name.str + start_idx + 1;
          current_shader_name.size = final_idx - (start_idx + 2);
          if(lofi::str_compare(search_name, current_shader_name)) {
            return i;
          }
        }
        RX_ERRORF("failed to find shader named %s", shader_name);
        return MAX_u64;
      }

    }		// -----  end of namespace helpers  ----- 

   
      static u64 total_pixel_size() {
        // TODO:: finish
        return 0;
      }

      static u64 texture_count() {
        // TODO:: finish
        return 0;
      }

    static Extent texture_size(u64 texture_index) {
      // TODO:: finish
      return {};
    }

    static String texture_name(u64 texture_index) {
      // TODO:: finish
      return String{};
    }

    static PixelType texture_type_id(u64 texture_index) {
      // TODO:: finish
      return PixelType::Max;
    }

    static float* texture_pixels(u64 texture_index) {
      // TODO:: finish
      return nullptr;
    }

    static VkFormat texture_format(u64 texture_index) {
      // TODO:: finish
      return VK_FORMAT_MAX_ENUM;
    }


    static u64 mesh_count() {
      return (u64)graphics::resources::num_objs;
    }

    static u64 vertex_count() {
      return graphics::resources::total_num_vertices;
    }

    static u64 vertex_index_count() {
      return graphics::resources::total_num_indices;
    }

    static f32* get_vertices() {
      // TODO :: fix this
      return (f32*)graphics::resources::vertices;

    }

    static u64 mesh_vertex_count(u64 mesh_index) {
      return graphics::resources::obj_sizes[mesh_index];
    }

    static u64 mesh_index_count(u64 mesh_index) {
      return graphics::resources::obj_index_count[mesh_index];
    }

    static String mesh_name(u64 mesh_index) {
      return lofi::str_cstring(graphics::resources::obj_names[mesh_index]);
    }

    static const f32* mesh_vertices(u64 mesh_index) {
      return graphics::resources::vertices[graphics::resources::obj_handles[mesh_index]];
    }

    static const u32* mesh_indices(u64 mesh_index) {
      return static_cast<const u32*>(graphics::resources::indices[graphics::resources::obj_handles[mesh_index]]);
    }

    static u64 shader_count() {
      return config::shaders::TotalNumShaders;
    }

    static String shader_name(u64 shader_index) {
      return config::shaders::shader_names[shader_index];
    }

    static VkPipelineStageFlags2 shader_stage(u64 shader_index) {
      return config::shaders::shader_stages[shader_index];
    }

    static u64 shader_descriptor_set_count(u64 shader_index) {
      return config::shaders::shader_descriptor_set_counts[shader_index];
    }

    static u64 shader_descriptor_set_count(const char* shader_name) {
      return shader_descriptor_set_count(
          helpers::find_shader_index_from_shader_name(shader_name)
          );
    }


    static u64 shader_descriptor_set_handle(u64 shader_index, u64 descriptor_set_index) {
      // TODO
      return 0;
    }

    static u64 shader_descriptor_set_binding_count(u64 shader_index, u64 descriptor_set_index) {
      if(descriptor_set_index < shader_descriptor_set_count(shader_index)) {
        PRINT("[ERROR] descriptor_set_index = %llu must be higher than descriptor_set_count = %llu for the shader being indexed in rx_resource_manager.hpp", descriptor_set_index, shader_descriptor_set_count(shader_index));
        return MAX_u64;
      }
      u64 accumulated_index = 0;
      for(u64 i = 0; i < shader_index; i++) {
        accumulated_index += shader_descriptor_set_count(i);
      }
      return config::shaders::shader_descriptor_set_binding_counts[accumulated_index + descriptor_set_index];
    }

    static VkDescriptorType shader_descriptor_set_binding_type(u64 shader_index, u64 descriptor_set_index, u64 descriptor_set_binding_index) {
      // TODO
      u64 accumulated_index = 0;
      for(u64 i = 0; i < shader_index; i++) {
        for(u64 j = 0; j < descriptor_set_index; j++) {
          accumulated_index += shader_descriptor_set_binding_count(i, j);
        }
      }
      return config::shaders::shader_descriptor_set_binding_types[accumulated_index + descriptor_set_binding_index];
    }

    static const File shader_spirv_file(u64 shader_index) {
      File result = File::create<FileType::ReadOnly>(config::shaders::shader_names[shader_index]);
      return result;
    }

//    static u32* shader_spirv_data(u64 shader_index) {
//      // TODO:: add spirv to config::resources::shaders
//
//
//
//      return nullptr;
//    }

    static u64 sample_count() {
      return audio::samples::num_samples;
    }

    static u64 sample_frame_count(u64 sample_index) {
      return audio::samples::sample_sizes[sample_index];
    }

    static String sample_name(u64 sample_index) {
      return lofi::str_cstring(audio::samples::sample_names[sample_index]);
    }

    static const u32* sample_frames(u64 sample_index) {
      return 
        audio::samples::samples 
        + audio::samples::sample_handles[sample_index];
    }

  }		// -----  end of namespace resource  ----- 
}		// -----  end of namespace roxi  ----- 
