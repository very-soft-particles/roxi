// =====================================================================================
//
//       Filename:  vk_allocation_callbacks.h
//
//    Description:  
//
//        Version:  1.0
//        Created:  2023-12-08 1:14:08 PM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Roxi Graves (rg), nada
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#pragma once
#if OS_WINDOWS
#include <vulkan/vulkan_win32.h>
#elif OS_LINUX
#include <vulkan/vulkan_xcb.h>
#elif OS_ANDROID
#include <vulkan/vulkan_android.h>
#else 
#include <vulkan/vulkan.h>
#endif

namespace roxi {
  namespace vk {

    class AllocCallbacks {
    public:
//      using allocation_block_sizes_t = mem::bucket_descriptors_t<ID>;
//      using allocator_t = mem::MemoryPool<ID>;
//      static constexpr size_t num_allocation_block_sizes = mem::bucket_count<ID>;
      
      void init();
      void terminate();
      VkAllocationCallbacks* get_instance() { return &m_callbacks; }
      //void clear();

    private:
      // const allocator_t& _allocator;
      VkAllocationCallbacks m_callbacks;
//      Arena<MB(1), 64, mem::MAllocPolicy> m_arena;

      static void* allocate1(size_t size, size_t alignment);
      static void deallocate1(void* ptr);
      static void* reallocate1(void* ptr, size_t size, size_t align);
      
      //static void* allocate2(size_t size, size_t alignment);
      //static void deallocate2(void* ptr);
      //static void* reallocate2(void* ptr, size_t size, size_t align);

      //static void* allocate3(size_t size, size_t alignment);
      //static void deallocate3(void* ptr);
      //static void* reallocate3(void* ptr, size_t size, size_t align);

      static void* VKAPI_CALL alloc(void* data, size_t size, size_t alignment, VkSystemAllocationScope scope);
      static void VKAPI_CALL dealloc(void* data, void* ptr);
      static void* VKAPI_CALL realloc(void* data, void* ptr, size_t size, size_t align, VkSystemAllocationScope scope);
      static void VKAPI_CALL allocate_internal(void* data, unsigned long long size, VkInternalAllocationType type, VkSystemAllocationScope scope);
      static void VKAPI_CALL deallocate_internal(void* data, unsigned long long size, VkInternalAllocationType type, VkSystemAllocationScope scope);

    };

    VkAllocationCallbacks* get_vk_callbacks();
//    template<size_t ID>
//    void* AllocCallbacks<ID>::allocate2(size_t size, size_t alignment) {
//      //std::cout << "allocate2 in vk callback" << std::endl;
//      void* ptr = m_arena.push(size + sizeof(size_t));
//      MEM_COPY(ptr, &size, size);
//      if( ptr == nullptr 
//          /*  || ptr != mem::helpers::align_ptr(ptr, alignment  alignment was too high ) */  ) {
//        throw std::runtime_error("bad alloc in vulkan allocation callbacks allocate2");
//      }
//      return (void*)(((uint8_t*)ptr) + sizeof(size_t));
//    }
//
//    template<size_t ID>
//      void AllocCallbacks<ID>::deallocate2(void* ptr) {}
//
//    template<size_t ID>
//    void* AllocCallbacks<ID>::reallocate2(void* ptr, size_t size, size_t alignment) {
//      //std::cout << "reallocate2 in vk callback" << std::endl;
//      void* new_ptr = RuntimeAllocator<ID>::allocate(size);
//      if( new_ptr == nullptr ) {
//        throw std::runtime_error("bad realloc in vulkan allocation callbacks reallocate2");
//      }
//      size_t old_size = *(((uint8_t*)ptr) - sizeof(size_t));
//      size = MIN(size, old_size);
//      MEM_COPY(new_ptr, ptr, size);
//      return new_ptr;
//    }
//
//    template<size_t ID>
//    void* AllocCallbacks<ID>::allocate3(size_t size, size_t alignment) {
//      //std::cout << "allocate2 in vk callback" << std::endl;
//      void* ptr = _aligned_malloc(size, alignment);
//      if( ptr == nullptr ) {
//        throw std::runtime_error("bad alloc in vulkan allocation callbacks allocate2");
//      }
//      return ptr;
//    }
//
//    template<size_t ID>
//      void AllocCallbacks<ID>::deallocate3(void* ptr) {
//        //std::cout << "deallocate2 in vk callback" << std::endl;
//        _aligned_free(ptr);
//      }
//
//    template<size_t ID>
//    void* AllocCallbacks<ID>::reallocate3(void* ptr, size_t size, size_t alignment) {
//      //std::cout << "reallocate2 in vk callback" << std::endl;
//      void* new_ptr = m_arena.push(size);
//      if( new_ptr == nullptr ) {
//        throw std::runtime_error("bad realloc in vulkan allocation callbacks reallocate2");
//      } 
//      return new_ptr;
//    }
//    template<size_t ID>
//    void AllocCallbacks<ID>::clear() {
//      m_arena.clear();
//    }

  }		// -----  end of namespace vk  ----- 
}		// -----  end of namespace roxi  ----- 
