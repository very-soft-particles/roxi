// =====================================================================================
//
//       Filename:  rx_allocator.hpp
//
//    Description:  allocation defines 
//
//        Version:  1.0
//        Created:  2024-09-17 11:34:30 AM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Roxi Graves (rg), nada
//   Organization:  Roxi Psychotronics
//
// ====================================================================================
#pragma once
#include "rx_vocab.h"
#include "error.h"
#include "../../../lofi/core/include/l_allocator.hpp"

namespace roxi {

  static constexpr u64 LockFreeAllocatorPoolID =        0;

  using StaticAllocator = lofi::StaticAllocator;

  template<size_t AllocID>
  using RuntimeAllocator = lofi::RuntimeAllocator<AllocID>;

  using LockFreeStaticAllocator = lofi::LockFreeStaticAllocator;

  using LockFreeRuntimeAllocator = lofi::LockFreeRuntimeAllocator<LockFreeAllocatorPoolID>;

  template<size_t Size>
  using Block = lofi::mem::Block<Size>;

  namespace mem {
     
    static void* _allocate(u64 thread_id, u64 size) {
      lofi::dispatcher _dispatch
      { lofi::IdxV<RoxiNumThreads>
      , [size]<u64 ID>(lofi::IdxT<ID>) {
        return roxi::RuntimeAllocator<ID>::allocate(size);
      }};
      return _dispatch(thread_id);
    }

    static void _free(u64 thread_id, void* ptr) {
      lofi::dispatcher _dispatch
      { lofi::IdxV<RoxiNumThreads>
      , [ptr]<u64 ID>(lofi::IdxT<ID>) {
        return roxi::RuntimeAllocator<ID>::free(ptr);
      }};
      for(u64 i = thread_id; thread_id < RoxiNumThreads; thread_id++) {
        const u64 id = i & (RoxiNumThreads - 1);
        if(_dispatch(id)) {
          return;
        }
      }
      RX_ASSERT(false, "failed to find appropriate pool to free from");
    }

    static void* _reallocate(u64 thread_id, void* src, u64 size) {
      lofi::dispatcher _dispatch
      { lofi::IdxV<RoxiNumThreads>
      , [src, size]<u64 ID>(lofi::IdxT<ID>) {
        return roxi::RuntimeAllocator<ID>::reallocate(src, size);
      }};
      for(u64 i = thread_id; thread_id < RoxiNumThreads; thread_id++) {
        const u64 id = i & (RoxiNumThreads - 1);
        void* result = _dispatch(id);
        if(result != nullptr) {
          return result;
        }
      }
      RX_ASSERT(false, "failed to find apppropriate pool to reallocate from");
      return nullptr;
    }

    static void* _allocate_aligned(u64 thread_id, u64 size, u64 alignment) {
      lofi::dispatcher _dispatch
      { lofi::IdxV<RoxiNumThreads>
      , [size, alignment]<u64 ID>(lofi::IdxT<ID>) {
        return roxi::RuntimeAllocator<ID>::allocate(size, alignment);
      }};
      return _dispatch(thread_id);
    }

    static void* _reallocate_aligned(u64 thread_id, void* src, u64 size, u64 alignment) {
      lofi::dispatcher _dispatch
      { lofi::IdxV<RoxiNumThreads>
      , [src, size, alignment]<u64 ID>(lofi::IdxT<ID>) {
        return roxi::RuntimeAllocator<ID>::reallocate(src, size, alignment);
      }};
      for(u64 i = thread_id; thread_id < RoxiNumThreads; thread_id++) {
        const u64 id = i & (RoxiNumThreads - 1);
        void* result = _dispatch(id);
        if(result != nullptr) {
          return result;
        }
      }
      RX_ASSERT(false, "failed to find apppropriate pool to reallocate from");
      return nullptr;
    }
  
    static void* _lock_free_allocate(u64 size) {
      return roxi::LockFreeRuntimeAllocator::allocate(size);
    }

    static void _lock_free_free(void* ptr) {
      roxi::LockFreeRuntimeAllocator::free(ptr);
    }

    static void* _lock_free_reallocate(void* src, u64 size) {
      return roxi::LockFreeRuntimeAllocator::reallocate(src, size);
    }

    static void* _lock_free_allocate_aligned(u64 size, u64 alignment) {
      return roxi::LockFreeRuntimeAllocator::allocate(size, alignment);
    }

    static void* _lock_free_reallocate_aligned(void* src, u64 size, u64 alignment) {
      return roxi::LockFreeRuntimeAllocator::reallocate(src, size, alignment);
    }
  }		// -----  end of namespace mem  ----- 

}		// -----  end of namespace roxi  ----- 

#if defined(RX_USE_LOCK_FREE_MEMORY)

  #define ALLOCATE(size) roxi::mem::_lock_free_allocate(static_cast<u64>(size))
  #define FREE(ptr) roxi::mem::_lock_free_free(static_cast<void*>(ptr))
  #define REALLOCATE(src, size) roxi::mem::_lock_free_reallocate(static_cast<void*>(src))
  #define ALLOCATE_ALIGNED(size, alignment) roxi::mem::_lock_free_allocate_aligned(static_cast<u64>(size), static_cast<u64>(alignment))
  #define REALLOCATE_ALIGNED(src, size, alignment) roxi::mem::_lock_free_reallocate_aligned(static_cast<void*>(src), static_cast<u64>(size), static_cast<u64>(alignment))

#elif defined(RX_USE_PER_THREAD_MEMORY)

  #define ALLOCATE(size) roxi::mem::_allocate(GET_HOST_THREAD_ID(ThreadPool), static_cast<u64>(size))
  #define FREE(ptr) (ptr) ? roxi::mem::_free(GET_HOST_THREAD_ID(ThreadPool), static_cast<void*>(ptr)) : 
  #define REALLOCATE(src, size) roxi::mem::_reallocate(GET_HOST_THREAD_ID(ThreadPool), static_cast<void*>(src))
  #define ALLOCATE_ALIGNED(size, alignment) roxi::mem::_allocate_aligned(GET_HOST_THREAD_ID(ThreadPool), static_cast<u64>(size), static_cast<u64>(alignment))
  #define REALLOCATE_ALIGNED(src, size, alignment) roxi::mem::_reallocate_aligned(GET_HOST_THREAD_ID(ThreadPool), static_cast<void*>(src), static_cast<u64>(size), static_cast<u64>(alignment))

#else

  #define ALLOCATE(size) malloc(static_cast<u64>(size))
  #define FREE(ptr) (ptr) ? free(static_cast<void*>(ptr)) : (void)0
  #define REALLOCATE(src, size) realloc(static_cast<void*>(src), static_cast<u64>(size))
  #define ALLOCATE_ALIGNED(size, alignment) _aligned_malloc(static_cast<u64>(size), static_cast<u64>(alignment))
  #define REALLOCATE_ALIGNED(src, size, alignment) _aligned_realloc(static_cast<void*>(src), static_cast<u64>(size), static_cast<u64>(alignment))

#endif

