// =====================================================================================
//
//       Filename:  vk_allocator.hpp
//
//    Description:  
//
//        Version:  1.0
//        Created:  2024-04-27 3:04:32 PM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Roxi Graves (rg), nada
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#pragma once
#include "vk_consts.h"
#include "vk_context.h"
#include <vulkan/vulkan_core.h>

namespace roxi {
  namespace vk {
    namespace mem {
      // base allocation, offset into allocation
      struct Allocation {
        VkDeviceMemory data = VK_NULL_HANDLE;
        VkDeviceSize offset = 0;
      };

      class MemoryArena {
      private:
        VkDeviceMemory _data = VK_NULL_HANDLE;
        VkDeviceSize _cap = 0;
#if defined (RX_USE_VK_LOCK_FREE_MEMORY)
        Counter _top{0};
#else 
        VkDeviceSize _top = 0;
#endif

      public:
        MemoryArena() {}

        b8 init(Context* context, const VkDeviceSize new_size, const u32 type_index, const VkMemoryAllocateFlags allocate_flags);
        b8 terminate(Context* context);
        VkDeviceSize push(const VkDeviceSize num_bytes);
        b8 pop_to(const VkDeviceSize offset);
        void clear();

        u64 get_size() {
#if defined (RX_USE_VK_LOCK_FREE_MEMORY)
          return _top.get_count();
#else
          return _top;
#endif
        }

        u64 get_cap() {
          return _cap;
        }

        operator bool();

        b8 operator==(const MemoryArena& other);

        VkDeviceMemory get_data() const {
          return _data;
        }
      };

      // size, type_index, flags
      struct ArenaDescriptor { 
        u64 size = MAX_u64; 
        u64 type_index = MAX_u64;
        VkMemoryPropertyFlags flags = 0;
        VkMemoryAllocateFlags allocate_flags = 0;
      };

      using ArenaDescriptorSet = Array<ArenaDescriptor>;

      class MemoryPoolBuilder;

      class MemoryPool {
      public:
        // one for each memory type
      private:
        friend class MemoryPoolBuilder;
        Array<MemoryArena> arenas;
        u16 frame_id = MAX_u16;
      public:
        MemoryPool();
        b8 init(const u64 arena_count);
        b8 terminate(Context* context);

        Allocation allocate(const u32 bucket_index, const u32 num_bytes, const u32 alignment = 0);
        b8 free_to(u32 arena_index, VkDeviceSize offset);
        void clear_arena(u32 arena_index);
        u32 get_arena_count();
        VkDeviceMemory get_memory_handle(u32 arena_index);

      private:
        u32 initialize_bucket(Context* context, ArenaDescriptor descriptor);
      };

      class MemoryPoolBuilder {
      public:
      private:
        Context* vk_context = nullptr;
        VkPhysicalDeviceMemoryProperties2 memory_properties{};
        VkPhysicalDeviceProperties2 device_properties{};
        VkPhysicalDeviceDescriptorBufferPropertiesEXT descriptor_buffer_props{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_PROPERTIES_EXT};
        ArenaDescriptorSet bucket_descriptors;
        Arena arena;
        const u32 find_memory_type_index(VkMemoryPropertyFlags flags, const u32 memory_type_bits = MAX_u32);
      public:
        MemoryPoolBuilder();
        b8 init(Context* context);
        b8 terminate();

        u32 register_bucket(VkDeviceSize block_size, VkMemoryPropertyFlags flags, VkMemoryAllocateFlags allocate_flags = 0);

        u32 register_bucket(VkMemoryRequirements mem_reqs, VkMemoryPropertyFlags flags, VkMemoryAllocateFlags allocate_flags = 0);

        const VkPhysicalDeviceDescriptorBufferPropertiesEXT& get_descriptor_buffer_properties();
        b8 build(MemoryPool* pool);
      };
    }		// -----  end of namespace mem  ----- 
  }		// -----  end of namespace vk  ----- 
}		// -----  end of namespace roxi  ----- 

