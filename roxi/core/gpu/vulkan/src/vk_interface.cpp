// =====================================================================================
//
//       Filename:  vk_interface.cpp
//
//    Description:  
//
//        Version:  1.0
//        Created:  2024-06-26 7:52:28 PM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Roxi Graves (rg), nada
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#include "../include/vk_interface.hpp"
#include "rx_vocab.h"
#include "vk_interface.hpp"
#include <vulkan/vulkan_core.h>


namespace roxi {
  namespace vk {

    b8 InterfaceBuilder::init(Interface* i_ptr, void* window) {
      _interface = i_ptr;
      return _interface->_context.init(window);
    }

    b8 Interface::terminate() {
      return _context.terminate();
    }

    template<VkBufferUsageFlags Flags, VkMemoryPropertyFlags MemoryFlags>
    Buffer InterfaceBuilder::add_buffer(u32 size, u32 count) {
      Buffer buffer{&_interface->_context};
      buffer.init((VkDeviceSize)size, Flags);
      buffer.register_bucket(&_memory_builder, MemoryFlags, count);
      return buffer;
    }
 
    template<u8 Dimension, VkImageUsageFlags Flags, VkFormat Format, VkSampleCountFlags SampleFlags, VkMemoryPropertyFlags MemoryFlags>
    Image<Dimension> InterfaceBuilder::add_image(Vector<Dimension> size, u32 mip_level_count, u32 count) {
      Image<Dimension> image{&_interface->_context};
      Extent<Dimension> extent;
      MEM_COPY(&extent.value, &size, sizeof(Extent<Dimension>::type));
      VkFormat format;
      image.init(extent, Format, Flags, mip_level_count, SampleFlags);
      image.register_bucket(&_memory_builder, MemoryFlags, count);
      return image;
    }

  }		// -----  end of namespace vk  ----- 
}		// -----  end of namespace roxi  ----- 

