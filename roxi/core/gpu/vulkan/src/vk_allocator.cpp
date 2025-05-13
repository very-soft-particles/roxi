// =====================================================================================
//
//       Filename:  vk_allocator.cpp
//
//    Description:  
//
//        Version:  1.0
//        Created:  2024-05-02 4:54:51 PM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Roxi Graves (rg), nada
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#include "rx_allocator.hpp"
#include "vk_allocator.hpp"
#include <vulkan/vulkan_core.h>

namespace roxi {
  namespace vk {
    namespace mem {

      b8 MemoryArena::init(Context* context, const VkDeviceSize new_size, const u32 type_index, const VkMemoryAllocateFlags allocate_flags) {
        _cap = new_size;
        clear();
        RX_TRACE("initing vk mem arena");
        VkMemoryAllocateInfo alloc_info {};
        alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        alloc_info.memoryTypeIndex = type_index;
        alloc_info.allocationSize = _cap;

        VkMemoryAllocateFlagsInfo flags_info{};
        if(allocate_flags != 0) {
          RX_TRACE("adding allocate flags");
          flags_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
          if(allocate_flags & VK_MEMORY_ALLOCATE_DEVICE_MASK_BIT) {
            // device_mask = something
            RX_ERROR("did not set up multiple physical devices for allocate device mask");
            return false;
          } else {
            flags_info.deviceMask = 1;
          }
          flags_info.flags = allocate_flags;
          alloc_info.pNext = &flags_info;
        }

        RX_TRACEF("allocating vk memory arena cap = %llu", _cap);
        VK_CHECK(context->get_device().get_device_function_table().vkAllocateMemory(context->get_device().get_device(), &alloc_info, CALLBACKS(), &_data), "failed to allocate vk memory");
        return true;
      }

      b8 MemoryArena::terminate(Context* context) {
        context->get_device().get_device_function_table().vkFreeMemory(context->get_device().get_device(), _data, CALLBACKS());
        return true;
      }

      VkDeviceSize MemoryArena::push(const VkDeviceSize num_bytes) {
        RX_TRACEF("memory arena pushing %llu bytes, current top = %llu", num_bytes, _top);
#if defined (RX_USE_VK_LOCK_FREE_MEMORY)
        const u32 result = top.add((u64)num_bytes);
        const u32 new_top = result + num_bytes;
#else
        const VkDeviceSize result = _top;
        const VkDeviceSize new_top = _top + num_bytes;
#endif

        if(new_top > _cap) {
          RX_TRACEF("vk memory arena overflow cap = %llu, old_top = %llu, and new_top = %llu", _cap, _top, new_top);
          return (VkDeviceSize)MAX_u64;
        }
#ifndef RX_USE_VK_LOCK_FREE_MEMORY
        _top = new_top;
#endif
        return static_cast<VkDeviceSize>(result);
      }

      b8 MemoryArena::pop_to(const VkDeviceSize offset) {
        const u32 os = (u32)offset;
        if(os < get_size()) {
          _top = os;
          return true;
        }
        return false;
      }

      MemoryArena::operator bool() {
        if(_data == VK_NULL_HANDLE)
          return false;
        return true;
      }

      b8 MemoryArena::operator==(const MemoryArena& other) {
        if(other._data == _data)
          return true;
        return false;
      }

      void MemoryArena::clear() {
#if defined (RX_USE_VK_LOCK_FREE_MEMORY)
        top.reset();
#else
        _top = 0;
#endif
      }

      MemoryPool::MemoryPool() {}

      b8 MemoryPool::init(const u64 arena_count) {
        RX_CHECK(arena_count != 0
            , "attempted to create 0 memory arenas in vk memory pool");
        arenas.move_ptr(ALLOCATE(sizeof(MemoryArena) * arena_count));
        arenas.clear();
        RX_CHECK(arenas.get_buffer() != nullptr
            , "failed to create vk memory arenas");
        RX_END();
      }

      b8 MemoryPool::terminate(Context* context) {
        const u32 bucket_count = arenas.get_size();
        for(size_t i = 0; i < bucket_count; i++) {
          if(!arenas[i].terminate(context)) {
            return false;
          }
        }
        return true;
      }

      void MemoryPool::clear_arena(u32 arena_index) {
        arenas[arena_index].clear();
      }

//      b8 initialize_memory_pool(MemoryPoolBuilder* builder, const u64 frame_index) {
//        return builder->build(&s_memory_pools[frame_index]);
//      }
//
//      b8 terminate_memory_pool(Context* context, const u64 frame_index) {
//        return s_memory_pools[frame_index].terminate(context);
//      }

      Allocation MemoryPool::allocate(const u32 arena_index, const u32 num_bytes, const u32 alignment) {
        RX_TRACEF("allocating %u at arena index = %u", num_bytes, arena_index);
        Allocation new_allocation{};
        MEM_ZERO(&new_allocation, sizeof(Allocation));
        MemoryArena& arena = arenas[arena_index];
        new_allocation.data = arena.get_data();
        new_allocation.offset = arena.push(num_bytes + alignment);
        new_allocation.offset = ALIGN_POW2(new_allocation.offset, alignment);
        return new_allocation;
      }

      b8 MemoryPool::free_to(const u32 arena_index, VkDeviceSize offset) {
        return arenas[arena_index].pop_to(offset);
      }

      VkDeviceMemory MemoryPool::get_memory_handle(const u32 arena_index) {
        if(arena_index < arenas.get_size()) {
          return arenas[arena_index].get_data();
        }
        return VK_NULL_HANDLE;
      }

      u32 MemoryPool::get_arena_count() {
        return arenas.get_size();
      }

      u32 MemoryPool::initialize_bucket(Context* context, ArenaDescriptor descriptor) {
        const u32 result = (u32)arenas.get_size();
        RX_TRACEF("initializing memory arena with size = %llu, type index = %llu, and allocate flags = %u", descriptor.size, descriptor.type_index, descriptor.allocate_flags);
        arenas.push(1)->init(context, descriptor.size, descriptor.type_index, descriptor.allocate_flags);
        return result;
      }

      b8 MemoryPoolBuilder::init(Context* context) {
        vk_context = context;
        memory_properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2;
        context->get_instance()
          .get_instance_function_table()
          .vkGetPhysicalDeviceMemoryProperties2(
            context->get_device()
              .get_physical_device()
            , &memory_properties);

        device_properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
        device_properties.pNext = &descriptor_buffer_props;
        context->get_instance()
          .get_instance_function_table()
          .vkGetPhysicalDeviceProperties2(
            context->get_device()
              .get_physical_device()
            , &device_properties);

        arena.move_ptr(ALLOCATE(KB(4)));
        bucket_descriptors.move_ptr(arena.push(KB(4)));
        return true;
      }

      MemoryPoolBuilder::MemoryPoolBuilder() : arena(nullptr), bucket_descriptors(nullptr) {}


      b8 MemoryPoolBuilder::terminate() {
        FREE((void*)arena.get_buffer());
        return true;
      }

      u32 MemoryPoolBuilder::register_bucket(VkDeviceSize block_size, VkMemoryPropertyFlags flags, VkMemoryAllocateFlags allocate_flags) {
        const u64 current_descriptor_count = bucket_descriptors.get_size();
        const u32 page_size = device_properties.properties.limits.bufferImageGranularity;
        const u32 type_index = find_memory_type_index(flags);
        RX_RETURN(type_index != MAX_u32
            , "failed to find appropriate memory type index"
            , MAX_u32);
        const u32 new_block_size = ((block_size / page_size) + 1) * page_size;
        RX_TRACEF("allocation page size = %u, type index = %u, block size = %u, current descriptor count = %llu", page_size, type_index, new_block_size, current_descriptor_count);
        for(size_t i = 0; i < current_descriptor_count; i++) {
          ArenaDescriptor& current_descriptor = bucket_descriptors[i];
          if((current_descriptor.type_index == type_index) 
              && ((flags & current_descriptor.flags) == flags)
              && ((allocate_flags & current_descriptor.allocate_flags) == allocate_flags)) {
            if((new_block_size + current_descriptor.size) 
                < memory_properties.memoryProperties.memoryHeaps[memory_properties.memoryProperties.memoryTypes[type_index].heapIndex].size) {
              RX_TRACE("found memory type, enlarging allocation size");
              current_descriptor.size += new_block_size;
              return (u32)i;
            }
          }
        }
        RX_TRACE("creating new bucket");
        VkMemoryType current_type = memory_properties.memoryProperties.memoryTypes[type_index];
        if(memory_properties.memoryProperties.memoryHeaps[current_type.heapIndex].size
          > (new_block_size)) {

          u32 result = bucket_descriptors.get_size();
          ArenaDescriptor* descriptor = bucket_descriptors.push(1);
          descriptor->size = new_block_size;
          descriptor->type_index = type_index;
          descriptor->flags = flags;
          descriptor->allocate_flags = allocate_flags;
          return result;
        }
        RX_TRACEF("memory heap size too small at type index %u", type_index);
        return MAX_u32;
      }


      u32 MemoryPoolBuilder::register_bucket(VkMemoryRequirements mem_reqs, VkMemoryPropertyFlags flags, VkMemoryAllocateFlags allocate_flags) {
        const u64 block_size = mem_reqs.size + mem_reqs.alignment;
        const u64 current_descriptor_count = bucket_descriptors.get_size();
        const u32 page_size = device_properties.properties.limits.bufferImageGranularity;
        const u32 type_index = find_memory_type_index(flags, mem_reqs.memoryTypeBits);
        RX_RETURN(type_index != MAX_u32
            , "failed to find appropriate memory type index"
            , MAX_u32);
        const u32 new_block_size = ((block_size / page_size) + 1) * page_size;
        RX_TRACEF("allocation page size = %u, type index = %u, block size = %u, current descriptor count = %llu", page_size, type_index, new_block_size, current_descriptor_count);
        for(size_t i = 0; i < current_descriptor_count; i++) {
          ArenaDescriptor& current_descriptor = bucket_descriptors[i];
          if((current_descriptor.type_index == type_index) 
              && ((flags & current_descriptor.flags) == flags)
              && ((allocate_flags & current_descriptor.allocate_flags) == allocate_flags)) {
            if((new_block_size + current_descriptor.size) 
                < memory_properties.memoryProperties.memoryHeaps[memory_properties.memoryProperties.memoryTypes[type_index].heapIndex].size) {
              
              RX_TRACE("found memory type, enlarging allocation size");
              current_descriptor.size += new_block_size;
              return (u32)i;
            }
          }
        }
        RX_TRACE("creating new bucket");
        VkMemoryType current_type = memory_properties.memoryProperties.memoryTypes[type_index];
        if(memory_properties.memoryProperties.memoryHeaps[current_type.heapIndex].size
          > (new_block_size)) {

          u32 result = bucket_descriptors.get_size();
          ArenaDescriptor* descriptor = bucket_descriptors.push(1);
          descriptor->size = new_block_size;
          descriptor->type_index = type_index;
          descriptor->flags = flags;
          descriptor->allocate_flags = allocate_flags;
          return result;
        }
        RX_TRACEF("memory heap size too small at type index %u", type_index);
        return MAX_u32;
      }
       
      const VkPhysicalDeviceDescriptorBufferPropertiesEXT& MemoryPoolBuilder::get_descriptor_buffer_properties() {
        return descriptor_buffer_props;
      }

      const u32 MemoryPoolBuilder::find_memory_type_index(VkMemoryPropertyFlags flags, const u32 memory_type_bits) {
        const u32 memory_type_count = memory_properties.memoryProperties.memoryTypeCount;
        for(u32 i = 0; i < memory_type_count; i++) {
          if((memory_properties.memoryProperties.memoryTypes[i].propertyFlags & flags) == flags
            && ((BIT(i) & memory_type_bits) != 0)
          ) {
            return i;
          }
        }

        RX_ERRORF("failed to find appropriate vk memory type for memory property flags = %u, and memory type bits = %u", flags, memory_type_bits);
        return MAX_u32;
      }

      b8 MemoryPoolBuilder::build(MemoryPool* pool) {
        const size_t descriptor_count = bucket_descriptors.get_size();
        pool->init(descriptor_count);
        RX_TRACEF("creating %llu memory buckets", descriptor_count);
        for(size_t i = 0; i < descriptor_count; i++) {
          RX_TRACE("attempting to create memory bucket");
          if(bucket_descriptors[i].size == 0) {
            RX_TRACE("size = 0 continuing...");
            continue;
          }

          RX_TRACEF("creating memory bucket size = %llu, type = %llu", bucket_descriptors[i].size, bucket_descriptors[i].type_index);
          RX_CHECK(i == pool->initialize_bucket(vk_context, bucket_descriptors[i]), "memory pool failed to build correctly, check descriptors and do not initialize until all descriptors have been added!");
        }
        return true;
      }

    }		// -----  end of namespace mem  ----- 
  }		// -----  end of namespace vk  ----- 
}		// -----  end of namespace roxi  ----- 
