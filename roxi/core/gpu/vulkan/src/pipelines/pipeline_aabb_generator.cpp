#include "../../include/pipelines/pipeline_aabb_generator.h"
#include "ResourcesClusterForward.h"
#include "../../include/vk_barrier.hpp"
#include "Configs.h"
#include "rx_resource_manager.hpp"
#include "vk_descriptors.hpp"
#include "vk_pipeline.hpp"
#include <vulkan/vulkan_core.h>

namespace roxi {
  namespace vk {
    
    VkComputePipelineCreateInfo pipeline_aabb_generator_create_info(Context* context, )
    {
      static constexpr const char* shader_name = "aabb_generator.comp";

      PipelineCreation<PipelineType::Compute> creation;

      DescriptorSetLayoutBuilder descriptor_layout_builder{};
      descriptor_layout_builder.init(context);
      descriptor_layout_builder.set_flags(
          VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT
          | VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT
          );

      const u64 shader_index = resource::helpers::find_shader_index_from_shader_name(shader_name);
      const VkShaderStageFlags stage_flags = resource::shader_stage(shader_index);
      VkDescriptorSetLayoutBinding binding{};
      binding.stageFlags = stage_flags;

      DescriptorSetLayout layouts[4];

      const u64 descriptor_set_count = resource::shader_descriptor_set_count(shader_index);

      RX_ASSERT(descriptor_set_count <= 4
          , "too many descriptor sets in aabb_generator.comp!");

      for(u64 i = 0; i < descriptor_set_count; i++) {
        descriptor_layout_builder.clear_bindings();
        const u64 binding_count = resource::shader_descriptor_set_binding_count(shader_index, i);
        for(u64 j = 0; j < binding_count; j++) {
          const VkDescriptorType type 
            = resource::shader_descriptor_set_binding_type
              ( shader_index
              , i
              , j
              );
          binding.binding = (u32)j;
          // binding.descriptorCount = ??;
          binding.descriptorType = type;
          if(type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {
            // binding.pImmutableSamplers = ??;
          }
          descriptor_layout_builder.add_binding(binding);
        }
        descriptor_layout_builder.build(layouts + i);
      }
      VkPipelineLayout pipeline_layout = create_pipeline_layout(context, layouts, descriptor_set_count, nullptr, 0);
      creation.set_layout(pipeline_layout);

      VkShaderModuleCreateInfo shader_create_info
      { VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO
      , nullptr
      , 0
      , resource::shader_spirv_size(shader_index)
      , resource::shader_spirv_data(shader_index)
      };

      VkShaderModule mod{};

      VK_ASSERT(context->get_device().get_device_function_table()
        .vkCreateShaderModule(context->get_device().get_device()
            , &shader_create_info
            , CALLBACKS()
            , &mod
            )
        , "failed to create aabb_generator.comp shader module");

      creation.set_shader(mod);

      return creation.get_create_info();
    }

    PipelineAABBGenerator::~PipelineAABBGenerator()
    {
      for (auto uboBuffer : cfUBOBuffers_)
      {
        uboBuffer.Destroy();
      }
    }

    void PipelineAABBGenerator::OnWindowResized(VulkanContext& ctx)
    {
      resourcesCF_->aabbDirty_ = true;
    }

    void PipelineAABBGenerator::FillCommandBuffer(VulkanContext& ctx, VkCommandBuffer commandBuffer)
    {
      uint32_t frameIndex = ctx.GetFrameIndex();
      if (!resourcesCF_->aabbDirty_)
      {
        return;
      }

      Execute(ctx, commandBuffer, frameIndex);

      resourcesCF_->aabbDirty_ = false;
    }

    void PipelineAABBGenerator::Execute(VulkanContext& ctx, VkCommandBuffer commandBuffer, uint32_t frameIndex)
    {
      vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_);

      vkCmdBindDescriptorSets(
          commandBuffer,
          VK_PIPELINE_BIND_POINT_COMPUTE,
          pipelineLayout_,
          0, // firstSet
          1, // descriptorSetCount
          &descriptorSets_[frameIndex],
          0, // dynamicOffsetCount
          0); // pDynamicOffsets

      ctx.InsertDebugLabel(commandBuffer, "PipelineFrustumCulling", 0xff99ff99);

      vkCmdDispatch(commandBuffer,
          static_cast<uint32_t>(ClusterForwardConfig::SliceCountX), // groupCountX
          static_cast<uint32_t>(ClusterForwardConfig::SliceCountY), // groupCountY
          static_cast<uint32_t>(ClusterForwardConfig::SliceCountZ)); // groupCountZ

      const VkBufferMemoryBarrier2 barrier = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2,
        .pNext = nullptr,
        .srcStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
        .srcAccessMask = VK_ACCESS_2_SHADER_WRITE_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
        .dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT,
        .srcQueueFamilyIndex = ctx.GetComputeFamily(),
        .dstQueueFamilyIndex = ctx.GetGraphicsFamily(),
        .buffer = resourcesCF_->aabbBuffer_.buffer_,
        .offset = 0,
        .size = resourcesCF_->aabbBuffer_.size_ };
      VulkanBarrier::CreateBufferBarrier(commandBuffer, &barrier, 1u);
    }

    void PipelineAABBGenerator::CreateDescriptor(VulkanContext& ctx)
    {
      constexpr uint32_t frameCount = AppConfig::FrameCount;

      VulkanDescriptorSetInfo dsInfo;
      dsInfo.AddBuffer(&(resourcesCF_->aabbBuffer_), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT); // 0
      dsInfo.AddBuffer(nullptr, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT); // 1

      // Pool and layout
      descriptorManager_.CreatePoolAndLayout(ctx, dsInfo, frameCount, 1u);

      // Sets
      for (size_t i = 0; i < frameCount; ++i)
      {
        dsInfo.UpdateBuffer(&(cfUBOBuffers_[i]), 1);

        descriptorManager_.CreateSet(ctx, dsInfo, &descriptorSets_[i]);
      }
    }
  }		// -----  end of namespace vk  ----- 
}		// -----  end of namespace roxi  ----- 
