// =====================================================================================
//
//       Filename:  vk_interface.hpp
//
//    Description:  a vulkan interface for interacting with the GPU 
//
//        Version:  1.0
//        Created:  2024-06-26 7:29:37 PM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Roxi Graves (rg), nada
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#pragma once
#include "rx_math.hpp"
#include "vk_context.h"
#include "vk_command.hpp"
#include "vk_allocator.hpp"
#include "vk_buffer.hpp"
#include "vk_image.hpp"
#include "vk_descriptors.hpp"
#include "vk_pipeline.hpp"
#include "vk_renderpass.hpp"
#include <vulkan/vulkan_core.h>

namespace roxi {
 
  namespace vk {

    class InterfaceBuilder;
    class Interface {
    private:
      friend class InterfaceBuilder;

#if (OS_WINDOWS)
      using Window = win::Window;
#else 
      using Window = void;
#endif
      Context _context;
    public:
      b8 init(Window* window);
      template<VkBufferUsageFlags Flags>
      Buffer create_buffer();

      template<u8 Dimension, VkImageUsageFlags Flags>
      Image<Dimension> create_image();

      VkDescriptorSetLayout create_descriptor_set_layout();
      RenderPass create_render_pass();
      Pipeline<PipelineType::Graphics> create_graphics_pipeline(RenderPass* render_pass);
      Pipeline<PipelineType::Compute> create_compute_pipeline();

      CommandBuffer create_command_buffer();

      b8 terminate();
    };

    class InterfaceBuilder {
    private:
      mem::MemoryPoolBuilder _memory_builder;
      Interface* _interface = nullptr;
    public:
      b8 init(Interface* i_ptr, void* window);

      template<VkBufferUsageFlags Flags, VkMemoryPropertyFlags MemoryFlags>
      Buffer add_buffer(u32 size, u32 count);

      template<u8 Dimension, VkImageUsageFlags Flags, VkFormat Format, VkSampleCountFlags SampleFlags, VkMemoryPropertyFlags MemoryFlags>
      Image<Dimension> add_image(Vector<Dimension> size, u32 mip_level_count, u32 count);

      RenderPass add_render_pass();

      template<PipelineType PipelineT>
      Pipeline<PipelineT> add_pipeline();

      VkDescriptorSetLayout add_descriptor_set_layout();
      CommandBuffer add_command_buffer();

      b8 build();
      Interface* get();
    };

  }		// -----  end of namespace vk  ----- 

}		// -----  end of namespace roxi  ----- 
