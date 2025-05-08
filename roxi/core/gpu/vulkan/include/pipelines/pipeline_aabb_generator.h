#pragma once
#include "../vk_pipeline.hpp"
/*
Generate voxelized frustum for clustered forward
*/
namespace roxi {
  namespace vk {
    
    VkComputePipelineCreateInfo create_pipeline_aabb_generator(ResourcesClusterForward* resources);

    void SetCameraUBO(VulkanContext& ctx, CameraUBO& ubo) override {}
    void FillCommandBuffer(VulkanContext& ctx, VkCommandBuffer commandBuffer) override;
    void OnWindowResized(VulkanContext& ctx) override;

    void SetClusterForwardUBO(VulkanContext& ctx, ClusterForwardUBO& ubo)
    {
      const size_t frameIndex = ctx.GetFrameIndex();
      cfUBOBuffers_[frameIndex].UploadBufferData(ctx, &ubo, sizeof(ClusterForwardUBO));
    }

    private:
    ResourcesClusterForward* resourcesCF_;

    Array<Buffer> cf_ubo_buffers_;
    std::array<VkDescriptorSet, AppConfig::FrameCount> descriptorSets_;

    private:
    void Execute(VulkanContext& ctx, VkCommandBuffer commandBuffer, uint32_t frameIndex);
    void CreateDescriptor(VulkanContext& ctx);

  }		// -----  end of namespace vk  ----- 
}		// -----  end of namespace roxi  ----- 
