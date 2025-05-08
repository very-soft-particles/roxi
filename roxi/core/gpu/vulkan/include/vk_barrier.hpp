// =====================================================================================
//
//       Filename:  vk_barrier.hpp
//
//    Description:  an abstraction for vulkan pipeline barriers, pilfered from Hello Vulkan github 
//
//        Version:  1.0
//        Created:  2024-08-18 9:01:52 AM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Roxi Graves (rg), nada
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#pragma once
#include "vk_allocation_callbacks.hpp"


namespace roxi {
  namespace vk {
      struct ImageBarrierInfo
      {
        VkCommandBuffer commandBuffer;

        VkImageLayout oldLayout;
        VkPipelineStageFlags2 sourceStage;
        VkAccessFlags2 sourceAccess;

        VkImageLayout newLayout;
        VkPipelineStageFlags2 destinationStage;
        VkAccessFlags2 destinationAccess;
      };

    // TODO this abstraction is still fairly simple
    class Barrier
    {
      public:
        static void CreateMemoryBarrier(VkCommandBuffer commandBuffer, const VkMemoryBarrier2* barriers, uint32_t barrierCount);
        static void CreateBufferBarrier(VkCommandBuffer commandBuffer, const VkBufferMemoryBarrier2* barriers, uint32_t barrierCount);
        static void CreateImageBarrier(const ImageBarrierInfo& info, const VkImageSubresourceRange& range, VkImage image);
        static void CreateImageBarrier(VkCommandBuffer commandBuffer, const VkImageMemoryBarrier2* barriers, uint32_t barrierCount);
    };
  }		// -----  end of namespace vk  ----- 
}		// -----  end of namespace roxi  ----- 
