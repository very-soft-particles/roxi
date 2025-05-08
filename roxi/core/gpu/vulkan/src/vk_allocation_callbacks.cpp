// =====================================================================================
//
//       Filename:  vk_allocation_callbacks.cpp
//
//    Description:   
//
//        Version:  1.0
//        Created:  2024-09-17 12:09:08 PM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Roxi Graves (rg), nada
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#include "../include/vk_allocation_callbacks.hpp"
#include "rx_allocator.hpp"

namespace roxi {
  namespace vk {

    void AllocCallbacks::init() {
      m_callbacks.pfnAllocation = alloc;
      m_callbacks.pfnFree = dealloc;
      m_callbacks.pfnReallocation = realloc;
      m_callbacks.pfnInternalAllocation = allocate_internal;
      m_callbacks.pfnInternalFree = deallocate_internal;
      m_callbacks.pUserData = (void*)this;
    }

    void AllocCallbacks::terminate() {
      m_callbacks.pfnAllocation = nullptr;
      m_callbacks.pfnFree = nullptr;
      m_callbacks.pfnReallocation = nullptr;
      m_callbacks.pfnInternalAllocation = nullptr;
      m_callbacks.pfnInternalFree = nullptr;
      m_callbacks.pUserData = nullptr;
    }

    void* AllocCallbacks::allocate1(size_t size, size_t alignment) {
      PRINT_LINE("allocating allocate1");
      return ALLOCATE_ALIGNED(size, alignment);
    }

    void AllocCallbacks::deallocate1(void* ptr) {
      PRINT_LINE("deallocating deallocate1");
      if(ptr)
        FREE(ptr);
    }

    void* AllocCallbacks::reallocate1(void* ptr, size_t size, size_t alignment) {
      PRINT_LINE("reallocating reallocate1");
      return REALLOCATE_ALIGNED(ptr, size, alignment);
    }

    VkAllocationCallbacks* get_vk_callbacks() {
      //PRINT_LINE("getting callbacks");
      static AllocCallbacks callbacks{};
      if(callbacks.get_instance()->pUserData == nullptr) {
        callbacks.init();
      }
      return callbacks.get_instance();
    }

    void* VKAPI_CALL AllocCallbacks::alloc(void* data, size_t size, size_t alignment, VkSystemAllocationScope scope) {
      // log something?
      //PRINT("allocating size %llu\n", size);
      void* ptr = static_cast<AllocCallbacks*>(data)->allocate1(size, alignment);
      //PRINT("allocated %llu size %llu\n", PTR2INT(ptr), size);
      return ptr;
//      return static_cast<AllocCallbacks*>(data)->allocate2(size, alignment);
    }

    void VKAPI_CALL AllocCallbacks::dealloc(void* data, void* ptr) {
//      if(!static_cast<AllocCallbacks*>(data)->m_arena->belongs(ptr)) 
      if(ptr == nullptr)
        return;
      //PRINT("freeing %llu\n", PTR2INT(ptr));
      static_cast<AllocCallbacks*>(data)->deallocate1(ptr);
      //PRINT("freed %llu\n", PTR2INT(ptr));
//      static_cast<AllocCallbacks*>(data)->deallocate2(ptr);
    }

    void* VKAPI_CALL AllocCallbacks::realloc(void* data, void* ptr, size_t size, size_t alignment, VkSystemAllocationScope scope) {
      // log something?
//      if(scope != VK_SYSTEM_ALLOCATION_SCOPE_COMMAND) {
//        if(!static_cast<AllocCallbacks*>(data)->m_arena->belongs(ptr)) {
      //PRINT("reallocating %llu\n", PTR2INT(ptr));
      return static_cast<AllocCallbacks*>(data)->reallocate1(ptr, size, alignment);
      //PRINT("reallocated %llu\n", PTR2INT(ptr));
//        }
//        return static_cast<AllocCallbacks*>(data)->reallocate2(ptr, size, alignment);
//      }
//      return static_cast<AllocCallbacks*>(data)->reallocate3(ptr, size, alignment);
    }

    void VKAPI_CALL AllocCallbacks::allocate_internal(void* data, unsigned long long size, VkInternalAllocationType type, VkSystemAllocationScope scope) {
    }

    void VKAPI_CALL AllocCallbacks::deallocate_internal(void* data, unsigned long long size, VkInternalAllocationType type, VkSystemAllocationScope scope) {
      // log something?
    }
  }
}

