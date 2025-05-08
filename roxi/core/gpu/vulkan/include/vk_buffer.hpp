// =====================================================================================
//
//       Filename:  vk_buffer.hpp
//
//    Description:  
//
//        Version:  1.0
//        Created:  2024-05-02 1:27:49 PM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Roxi Graves (rg), nada
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#pragma once
#include "vk_allocator.hpp"

#define VK_BUFFER_TYPES(X) X(HostUniformBuffer) X(DeviceUniformBuffer) X(HostStorageBuffer) X(DeviceStorageBuffer) X(StagingBuffer) X(IndirectCommand) X(TransferSource) X(DescriptorBuffer)

namespace roxi {
  namespace gpu {

#define ENUM(Type) Type, 
    enum class BufferType : u8 {
      VK_BUFFER_TYPES(ENUM)
      Max
    }; 
#undef ENUM
    static const char* get_vk_buffer_type_name(BufferType type) {
#define STR(TypeT) if (type == BufferType::TypeT) return (const char*)STRINGIFY(TypeT);
      VK_BUFFER_TYPES(STR)
#undef STR
        else
          return "";
    }
  }		// -----  end of namespace gpu  ----- 
  
  namespace vk {

    static VkBufferUsageFlags get_buffer_usage_flags(const gpu::BufferType type) {
      static const VkBufferUsageFlags _s_flags[] = 
        { VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT
        , VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT
        , VK_BUFFER_USAGE_STORAGE_BUFFER_BIT
        , VK_BUFFER_USAGE_STORAGE_BUFFER_BIT
        , VK_BUFFER_USAGE_TRANSFER_SRC_BIT
        , VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT
        , VK_BUFFER_USAGE_TRANSFER_DST_BIT
        };
      return _s_flags[(u8)type];
    }

    static constexpr u32 s_max_uniform_buffers  = 256;
    static constexpr u32 s_max_storage_buffers  = 256;
    static constexpr u32 s_max_staging_buffers =  64;
    static constexpr u32 s_max_buffers = s_max_staging_buffers + s_max_storage_buffers + s_max_uniform_buffers;

    class Buffer {
    private:
      VkBuffer _buffer = VK_NULL_HANDLE;
      VkDeviceSize _buffer_size = 0;
      gpu::BufferType _type;
      mem::Allocation _current_allocation{};
      u32 _alignment = 0;
    public:
      Context* _context = nullptr;
      b8 init
        ( Context* context
        , VkDeviceSize size
        , const gpu::BufferType type
        , VkBufferUsageFlags flags = 0
        );

      b8 bind(mem::Allocation memory);
      void* map() const;
      b8 unmap() const;
      b8 unbind();
      VkDeviceAddress get_device_address() const;
      const VkMemoryRequirements get_memory_requirements();
      b8 terminate();
      const u32 get_alignment() {
        return _alignment;
      }
      VkBuffer get_buffer() const;
      VkDeviceSize get_size() const;
      const gpu::BufferType get_buffer_type();
    };

  }		// -----  end of namespace vk  ----- 

}		// -----  end of namespace roxi  ----- 
