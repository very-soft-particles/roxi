#pragma once
#include "rx_vocab.h"
namespace roxi {
	namespace config {
		namespace shaders {
			inline constexpr u64 TotalNumShaders = 21;
			inline constexpr const char* shader_names[] = { "C:/roxi/roxi_0.8/roxi/core/resource/spirv/aabb_generator.comp.spv", "C:/roxi/roxi_0.8/roxi/core/resource/spirv/aabb_render.frag.spv", "C:/roxi/roxi_0.8/roxi/core/resource/spirv/aabb_render.vert.spv", "C:/roxi/roxi_0.8/roxi/core/resource/spirv/basic_shader.frag.spv", 
				"C:/roxi/roxi_0.8/roxi/core/resource/spirv/basic_shader.vert.spv", "C:/roxi/roxi_0.8/roxi/core/resource/spirv/build_skybox.frag.spv", "C:/roxi/roxi_0.8/roxi/core/resource/spirv/frustum_culling.comp.spv", "C:/roxi/roxi_0.8/roxi/core/resource/spirv/frustum_draw_indirect.comp.spv", 
				"C:/roxi/roxi_0.8/roxi/core/resource/spirv/fullscreen_triangle.vert.spv", "C:/roxi/roxi_0.8/roxi/core/resource/spirv/generate_partition.comp.spv", "C:/roxi/roxi_0.8/roxi/core/resource/spirv/light_culling.comp.spv", "C:/roxi/roxi_0.8/roxi/core/resource/spirv/light_culling_batch.comp.spv", 
				"C:/roxi/roxi_0.8/roxi/core/resource/spirv/light_orb.frag.spv", "C:/roxi/roxi_0.8/roxi/core/resource/spirv/light_orb.vert.spv", "C:/roxi/roxi_0.8/roxi/core/resource/spirv/line.frag.spv", "C:/roxi/roxi_0.8/roxi/core/resource/spirv/line.vert.spv", 
				"C:/roxi/roxi_0.8/roxi/core/resource/spirv/scene.frag.spv", "C:/roxi/roxi_0.8/roxi/core/resource/spirv/scene.vert.spv", "C:/roxi/roxi_0.8/roxi/core/resource/spirv/skybox.frag.spv", "C:/roxi/roxi_0.8/roxi/core/resource/spirv/skybox.vert.spv", 
				"C:/roxi/roxi_0.8/roxi/core/resource/spirv/tonemap.frag.spv"

			};

			inline constexpr u64 shader_descriptor_set_counts[] = {
        1, 0, 0, 0, 
				4, 0, 1, 2, 
				0, 2, 1, 1, 
				0, 1, 0, 1, 
				1, 1, 1, 1, 
				1
			};

			inline constexpr VkPipelineStageFlags2 shader_stages[] = {
( VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT ), ( VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT ), ( VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT ), ( VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT ), 
				( VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT ), ( VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT ), ( VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT ), ( VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT ), 
				( VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT ), ( VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT ), ( VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT ), ( VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT ), 
				( VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT ), ( VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT ), ( VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT ), ( VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT ), 
				( VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT ), ( VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT ), ( VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT ), ( VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT ), 
				( VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT )
			};
			inline constexpr u32 shader_descriptor_set_binding_counts[] = {
2, 
				2, 2, 2, 1, 
				3, 
				2, 4, 
				1, 1, 
				6, 
				6, 
				2, 
				2, 
				10, 
				3, 
				1, 
				1, 
				1
			};
			inline constexpr VkDescriptorType shader_descriptor_set_binding_types[] = {
VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 
				VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 
				VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 
				VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 
				VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 
				VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 
				VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 
				VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 
				VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 
				VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 
				VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 
				VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 
				VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 
				VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 
				VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 
				VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 
				VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 
				VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 
				VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 
				VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 
				VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 
				VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 
				VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
			};

		}		// -----  end of namespace shaders  ----- 
	} 	 // -----  end of namespace config  ----- 
}	  // -----  end of namespace roxi  ----- 
