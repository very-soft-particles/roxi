#pragma once
#include "rx_vocab.h"
namespace roxi {
	namespace config {
		namespace shaders {
			inline constexpr u64 TotalNumShaders = 2;
			inline constexpr const char* shader_names[] = { "C:/roxi/roxi/core/resource/spirv/basic_shader.frag.spv", "C:/roxi/roxi/core/resource/spirv/basic_shader.vert.spv"

			};

			inline constexpr u64 shader_descriptor_set_counts[] = {
0, 4
			};

			inline constexpr VkPipelineStageFlags2 shader_stages[] = {
( VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT ), ( VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT )
			};
			inline constexpr u32 shader_descriptor_set_binding_counts[] = {
2, 1, 2, 2
			};
			inline constexpr VkDescriptorType shader_descriptor_set_binding_types[] = {
VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 
				VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 
				VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 
				VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE
			};

		}		// -----  end of namespace shaders  ----- 
	} 	 // -----  end of namespace config  ----- 
}	  // -----  end of namespace roxi  ----- 
