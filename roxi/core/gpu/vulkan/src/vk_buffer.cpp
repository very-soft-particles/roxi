// =====================================================================================
//
//       Filename:  vk_buffer.cpp
//
//    Description:  
//
//        Version:  1.0
//        Created:  2024-05-05 9:37:38 AM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Roxi Graves (rg), nada
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#include "vk_buffer.hpp"
#include <vulkan/vulkan_core.h>

namespace roxi {
  
  namespace vk {


    b8 Buffer::init
      ( Context* context
      , VkDeviceSize size
      , const gpu::BufferType type
      , VkBufferUsageFlags flags
      ) 
    {
      RX_TRACEF("initializing buffer type %s with context at %llu", gpu::get_vk_buffer_type_name(type), PTR2INT(context));
      _context = context;
      _buffer_size = size;
      _type = type;

      //DeviceQueueIndex current_index;
      //u32 num_queue_families = 0;
      //  only for use with sharing mode concurrent, which we're not currently using
      //for(size_t i = 0; i < queue_count; i++) {
      //  current_index = vk_context->get_device().get_queue_family_index(queue_indices[i]);
      //  b8 to_continue = false;
      //  for(size_t j = 0; j < i; j++) {
      //    if(index_scratch[j] == current_index.get<0>()) {
      //      to_continue = true;
      //      break;
      //    }
      //  }
      //  if(to_continue) {
      //    continue;
      //  }
      //  *(index_scratch.push(1)) = vk_context->get_device().get_queue_family_index(queue_indices[i]).get<0>();
      //  num_queue_families++;
      //}

      VkBufferCreateInfo create_info{};
      create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
      create_info.pNext = nullptr;
      create_info.flags = 0;
      create_info.pQueueFamilyIndices = nullptr;
      create_info.queueFamilyIndexCount = 0;
      // if sharing mode exclusive, then leave indices null
      create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
      create_info.size = size;
      create_info.usage = type == gpu::BufferType::DescriptorBuffer ? flags : get_buffer_usage_flags(type);

      VK_CHECK
        ( _context->get_device()
          .get_device_function_table()
          .vkCreateBuffer
            ( _context->get_device().get_device()
            , &create_info
            , CALLBACKS()
            , &_buffer)
        , "failed to create vulkan buffer");
      return true;
    }

//    b8 Buffer::init
//      ( Context* context
//      , VkDeviceSize size
//      , VkBufferUsageFlags flags
//      ) 
//    {
//      _context = context;
//      _buffer_size = size;
//      _type = gpu::BufferType::Max;
//
//      //DeviceQueueIndex current_index;
//      //u32 num_queue_families = 0;
//      //  only for use with sharing mode concurrent, which we're not currently using
//      //for(size_t i = 0; i < queue_count; i++) {
//      //  current_index = vk_context->get_device().get_queue_family_index(queue_indices[i]);
//      //  b8 to_continue = false;
//      //  for(size_t j = 0; j < i; j++) {
//      //    if(index_scratch[j] == current_index.get<0>()) {
//      //      to_continue = true;
//      //      break;
//      //    }
//      //  }
//      //  if(to_continue) {
//      //    continue;
//      //  }
//      //  *(index_scratch.push(1)) = vk_context->get_device().get_queue_family_index(queue_indices[i]).get<0>();
//      //  num_queue_families++;
//      //}
//
//      VkBufferCreateInfo create_info{};
//      create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
//      create_info.pNext = nullptr;
//      create_info.flags = 0;
//      create_info.pQueueFamilyIndices = nullptr;
//      create_info.queueFamilyIndexCount = 0;
//      // if sharing mode exclusive, then leave indices null
//      create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
//      create_info.size = size;
//      create_info.usage = flags;
//
//      RX_TRACE("vkCreateBuffer");
//      VK_CHECK
//        ( _context->get_device()
//          .get_device_function_table()
//          .vkCreateBuffer
//            ( _context->get_device().get_device()
//            , &create_info
//            , CALLBACKS()
//            , &_buffer)
//        , "failed to create vulkan buffer");
//      return true;
//    }

    b8 Buffer::bind(mem::Allocation memory) {

      RX_TRACEF("binding memory at location %llu, with offset %u, context at %llu", PTR2INT(memory.data), (u32)memory.offset, PTR2INT(_context));


      VK_CHECK(_context->get_device()
        .get_device_function_table()
        .vkBindBufferMemory(
          _context->get_device().get_device()
          , _buffer
          , memory.data
          , memory.offset)
        , "failed to bind buffer");
      _current_allocation = memory;

      RX_TRACE("returning from bind buffer");
      return true;
    }

    const VkMemoryRequirements Buffer::get_memory_requirements() {
      VkBufferMemoryRequirementsInfo2 buffer_mem_reqs{VK_STRUCTURE_TYPE_IMAGE_MEMORY_REQUIREMENTS_INFO_2};
      buffer_mem_reqs.buffer = _buffer;
      VkMemoryRequirements2 mem_reqs{VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2};

      _context->get_device().get_device_function_table()
        .vkGetBufferMemoryRequirements2(_context->get_device().get_device()
            , &buffer_mem_reqs
            , &mem_reqs);

      return mem_reqs.memoryRequirements;
    }

    const gpu::BufferType Buffer::get_buffer_type() {
      return _type;
    }

    b8 Buffer::unbind() {
      _current_allocation.data = VK_NULL_HANDLE;
      _current_allocation.offset = MAX_u32;
      return true;
    }
    
    void* Buffer::map() const {
      if(_current_allocation.data == VK_NULL_HANDLE) {
        RX_ERROR("tried to map buffer that is not currently bound to any memory");
        return nullptr;
      }
      void* data = nullptr;
      if(_context->get_device().get_device_function_table().vkMapMemory(_context->get_device().get_device(), _current_allocation.data, _current_allocation.offset, _buffer_size, 0, &data) != VK_SUCCESS) {
        RX_ERROR("failed to properly map buffer");
        return nullptr;
      }
      return data;
    }

    b8 Buffer::unmap() const {
      _context->get_device().get_device_function_table().vkUnmapMemory(_context->get_device().get_device(), _current_allocation.data);
      return true;
    }

    VkDeviceAddress Buffer::get_device_address() const {
      VkBufferDeviceAddressInfo info
      { VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO
      , nullptr
      , _buffer
      };

      return _context->get_device().get_device_function_table()
          .vkGetBufferDeviceAddress
            ( _context->get_device().get_device()
            , &info
            );
    }

    b8 Buffer::terminate() {
      if(_buffer != VK_NULL_HANDLE) {
        _context->get_device()
          .get_device_function_table()
          .vkDestroyBuffer(
            _context->get_device().get_device()
            , _buffer
            , CALLBACKS());
        return true;
      }
      return false;
    }

    VkBuffer Buffer::get_buffer() const {
      return _buffer;
    }

    VkDeviceSize Buffer::get_size() const {
      return _buffer_size;
    }

  }		// -----  end of namespace vk  ----- 
}		// -----  end of namespace roxi  ----- 
