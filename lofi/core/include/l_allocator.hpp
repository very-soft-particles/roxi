// =====================================================================================
//
//       Filename:  l_allocator.hpp
//
//    Description:  
//
//        Version:  1.0
//        Created:  2025-02-14 8:30:33 PM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Robyn Gray (rg), ...
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#pragma once
#include "l_container.hpp"


namespace lofi {
 
// Bucket generation
  namespace mem {
    
      template<size_t BlockSize>
      struct Block {
        uint8_t block [BlockSize];
      };

    template<size_t BlockSize, size_t BlockCount, size_t Alignment>
      struct BucketDescriptor 
      {
        static constexpr size_t size = BlockSize;
        static constexpr size_t count = BlockCount;
        static constexpr size_t align = Alignment;
      };

    template<size_t ID>
      struct BucketDescriptorSet 
      {
        using type = List<>;
      };

    template<size_t ID>
      struct bucket_info {
        using bucket_descriptors_t = typename BucketDescriptorSet<ID>::type;

        static constexpr size_t bucket_count = list_size<bucket_descriptors_t>::value;

        template<typename Idx>
          struct block_info {
            static constexpr size_t block_size = meta::at_t<bucket_descriptors_t, Idx::value>::size;

            static constexpr size_t block_count = meta::at_t<bucket_descriptors_t, Idx::value>::count;

            static constexpr size_t block_align = meta::at_t<bucket_descriptors_t, Idx::value>::align;

            static constexpr size_t bucket_size = block_size * block_count;
          };

        template<typename... Idxs>
          using block_list_seq_t = List<Block<block_info<Idxs>::block_size>...>;

        template<typename... Idxs>
          using block_info_seq_t = List<block_info<Idxs>...>;

        template<typename... Idxs>
          using block_size_seq_t = List<IdxT<block_info<Idxs>::block_size>...>;

        template<typename... Idxs>
          using block_count_seq_t = List<IdxT<block_info<Idxs>::block_count>...>;

        template<typename... Idxs>
          using block_align_seq_t = List<IdxT<block_info<Idxs>::block_align>...>;

        template<typename... Idxs>
          using bucket_size_seq_t = List<IdxT<block_info<Idxs>::bucket_size + block_info<Idxs>::block_align>...>;

        using apply_block_info_seq = typename meta::lift<block_info_seq_t, typename IdxSequence<bucket_count>::type>;

        using apply_block_size_seq = typename meta::lift<block_size_seq_t, typename IdxSequence<bucket_count>::type>;

        using apply_block_count_seq = typename meta::lift<block_count_seq_t, typename IdxSequence<bucket_count>::type>;

        using apply_block_align_seq = typename meta::lift<block_align_seq_t, typename IdxSequence<bucket_count>::type>;

        using apply_bucket_size_seq = typename meta::lift<bucket_size_seq_t, typename IdxSequence<bucket_count>::type>;

        using apply_block_list_seq = typename meta::lift<block_list_seq_t, typename IdxSequence<bucket_count>::type>;

        static constexpr size_t set_size = meta::lift<meta::apply_sum, typename apply_bucket_size_seq::type>::type::value;

        static constexpr size_t set_count = meta::lift<meta::apply_sum, typename apply_block_count_seq::type>::type::value; 

        static constexpr size_t set_alignment = meta::lift<meta::apply_sum, typename apply_block_align_seq::type>::type::value; 

        template<typename Info>
          using Bucket = FreeListContainerPolicy<Block<Info::block_size>, Info::block_count, Info::block_align, SubAllocPolicy>;

        template<typename Info>
          using LockFreeBucket = lock_free_pool<Block<Info::block_size>, Info::block_count>;

        template<typename... Infos>
          using bucket_zip = List<Bucket<Infos>...>;

        using apply_bucket_zip = typename meta::lift<bucket_zip, typename apply_block_info_seq::type>;

        template<typename... Infos>
          using lock_free_bucket_zip = List<LockFreeBucket<Infos>...>;

        using lock_free_apply_bucket_zip = typename meta::lift<lock_free_bucket_zip, typename apply_block_info_seq::type>;
      };

    template<size_t ID>
      using make_pool = typename meta::lift<tuple_base, typename bucket_info<ID>::apply_bucket_zip::type>::type;

    template<size_t ID>
      using make_lock_free_pool = typename meta::lift<tuple_base, typename bucket_info<ID>::lock_free_apply_bucket_zip::type>::type;

    template<size_t ID>
      struct set_buckets {
        using info = bucket_info<ID>;
        static constexpr size_t num_buckets = info::bucket_count;
        template<size_t Index = 0>
          struct type {
            using block_info = typename info::template block_info<IdxT<Index>>;
            static constexpr size_t size = block_info::bucket_size + block_info::block_align;
            template<template<size_t> class PoolT>
              static void apply(PoolT<ID>* pool_ptr, void* ptr) {
                //PRINT("applying set pointers, ptr at: %llu\n", ptr);
                pool_ptr->template get<Index>().move_ptr(ptr);
                u8* new_ptr = (u8*)ptr + size;
                if constexpr (Index + 1 < num_buckets) {
                  type<Index + 1>::apply(pool_ptr, (void*)new_ptr);
                }
              }
          };
      };

    template<size_t ID>
      struct set_lf_buckets {
        using info = bucket_info<ID>;
        static constexpr size_t num_buckets = info::bucket_count;
        template<size_t Index = 0>
          struct type {
            using block_info = typename info::template block_info<IdxT<Index>>;
            static constexpr size_t size = block_info::bucket_size;
            template<template<size_t> class PoolT>
              static void apply(PoolT<ID>* pool_ptr, void* ptr) {
                //PRINT("applying set pointers, ptr at: %llu\n", ptr);
                pool_ptr->template get<Index>().set_ptr(ptr);
                u8* new_ptr = (u8*)ptr + size;
                if constexpr (Index + 1 < num_buckets) {
                  type<Index + 1>::apply(pool_ptr, (void*)new_ptr);
                }
              }
          };
      };

    template<size_t ID>
      struct StaticMemoryPool : StackAllocPolicy<bucket_info<ID>::set_size, 1024>
                                , make_pool<ID> 
    {
      using base_t = make_pool<ID>;
      using list_t = typename base_t::type;
      using alloc_t = StackAllocPolicy<bucket_info<ID>::set_size, 1024>;
      StaticMemoryPool() {
        set_pointers();
      }
      template<size_t Index>
        auto& get() {
          return static_cast<meta::at_t<list_t, Index>&>(*this);
        }
      private:
      void set_pointers() {
        u8* start = (u8*)alloc_t::data();
        set_buckets<ID>::template type<>::apply(this, start);
      }
    };

    template<size_t ID>
      struct HeapMemoryPool : MAllocPolicy<bucket_info<ID>::set_size, 256>
                              , make_pool<ID>
    {
      using base_t = make_pool<ID>;
      using list_t = typename base_t::type;
      using alloc_t = MAllocPolicy<bucket_info<ID>::set_size, 256>;
      HeapMemoryPool() {
        //PRINT_S("constructing heap memory pool\n");
        alloc_t::allocate();
        //PRINT_S("setting pointers\n");
        set_pointers();
      }

      template<size_t Index>
        auto& get() {
          return static_cast<meta::at_t<list_t, Index>&>(*this);
        }
      private:
      void set_pointers() {
        u8* start = (u8*)alloc_t::data();
        set_buckets<ID>::template type<>::apply(this, start);
      }
    };

    template<size_t ID>
      struct LockFreeStaticMemoryPool : StackAllocPolicy<bucket_info<ID>::set_size, 1024>
                                        , make_lock_free_pool<ID> 
    {
      using base_t = make_lock_free_pool<ID>;
      using list_t = typename base_t::type;
      using alloc_t = StackAllocPolicy<bucket_info<ID>::set_size, 1024>;
      using block_list_t = typename bucket_info<ID>::apply_block_list_seq;
      template<u64 I>
        using type_at_index = meta::at_t<block_list_t, I>;

      LockFreeStaticMemoryPool() {
        set_pointers();
      }
      template<size_t Index>
        auto& get() {
          return static_cast<meta::at_t<list_t, Index>&>(*this);
        }
      private:
      void set_pointers() {
        u8* start = (u8*)alloc_t::data();
        set_lf_buckets<ID>::template type<>::apply(this, start);
      }
    };

    template<size_t ID>
      struct LockFreeHeapMemoryPool : MAllocPolicy<bucket_info<ID>::set_size, 256>
                                      , make_lock_free_pool<ID>
    {
      using base_t = make_pool<ID>;
      using list_t = typename base_t::type;
      using alloc_t = MAllocPolicy<bucket_info<ID>::set_size, 256>;
      LockFreeHeapMemoryPool() {
        //PRINT_S("constructing heap memory pool\n");
        alloc_t::allocate();
        //PRINT_S("setting pointers\n");
        set_pointers();
      }

      template<size_t Index>
        auto& get() {
          return static_cast<meta::at_t<list_t, Index>&>(*this);
        }
      private:
      void set_pointers() {
        u8* start = (u8*)alloc_t::data();
        set_lf_buckets<ID>::template type<>::apply(this, start);
      }
    };

    template<size_t ID>
      auto& get_pool() {
        static StaticMemoryPool<ID> _pool{};
        return _pool;
      };

    template<size_t ID>
      auto& get_lock_free_pool() {
        static LockFreeStaticMemoryPool<ID> _pool{};
        return _pool;
      };

    namespace defaults {
      template<size_t Count>
        using Bucket_8 = BucketDescriptor<1 << 3, Count, 1 << 3>; // 16 byte aligned bucket
      template<size_t Count>
        using Bucket_16 = BucketDescriptor<1 << 4, Count, 1 << 4>; // 16 byte aligned bucket
      template<size_t Count>
        using Bucket_32 = BucketDescriptor<1 << 5, Count, 1 << 5>; // 32 byte aligned bucket
      template<size_t Count>
        using Bucket_64 = BucketDescriptor<1 << 6, Count, 1 << 6>; // 64 byte aligned bucket
      template<size_t Count>
        using Bucket_256 = BucketDescriptor<1 << 8, Count, 1 << 8>; // 256 byte aligned bucket

      template<size_t Count>
        using Bucket_1024 = BucketDescriptor<1 << 10, Count, 1 << 8>; // 1024 byte aligned bucket
      template<size_t Count>
        using Bucket_4096 = BucketDescriptor<1 << 12, Count, 1 << 8>; // 4096 byte aligned bucket
      template<size_t Count>
        using Bucket_16384 = BucketDescriptor<1 << 14, Count, 1 << 8>; // 16384 byte aligned bucket
      template<size_t Count>
        using Bucket_65536 = BucketDescriptor<1 << 16, Count, 1 << 8>; // 65536 byte aligned bucket
      template<size_t Count>
        using Bucket_256KB = BucketDescriptor<KB(256), Count, 1 << 8>; // 65536 byte aligned bucket
      template<size_t Count>
        using Bucket_1M = BucketDescriptor<MB(1), Count, 1 << 8>; // 1 mega-byte aligned bucket
      template<size_t Count>
        using Bucket_16M = BucketDescriptor<MB(16), Count, 1 << 8>; // 1 mega-byte aligned bucket
      template<size_t NumFibers>
        using FiberBucket = BucketDescriptor<KB(64) * NumFibers, 1, 1 << 8>; // 1 mega-byte aligned bucket

    }		// -----  end of namespace defaults  ----- 
  }		// -----  end of namespace mem  ----- 

  template<size_t ID>
  class HeapAllocator {
  public:
    HeapAllocator() : _pool() {}

    template<typename T>
    T* allocate() noexcept {
      static constexpr size_t N = sizeof(T);
      static constexpr size_t I = apply_find_bucket<N>::value;
      return (T*)FWD(_pool.template get<I>().add_object());
    }
    
    template<typename T>
    b8 deallocate(T* ptr) noexcept {
      static constexpr size_t N = sizeof(T);
      static constexpr size_t I = apply_find_bucket<N>::value;
      if(_pool.template get<I>().remove_object(static_cast<void*>(FWD(ptr))))
        return true;
      return false;
    }

    template<typename T, typename U>
    void* rebind(U* src) noexcept {
      auto new_ptr = allocate<ID, T>();
      L_ASSERT(new_ptr != nullptr);
      auto size = CLAMP_TOP(sizeof(T), sizeof(U));
      MEM_COPY(new_ptr, src, size);
      deallocate<ID, U>(src);
      return new_ptr;
    }

  private:
    mem::HeapMemoryPool<ID> _pool;


    template<size_t N>
    struct find_bucket {
      template<typename... Idxs>
      using type = typename meta::pred_find_t<meta::less_t<IdxT<N>>::template type>::template type<Idxs...>::type;
    };

    template<size_t N>
    using apply_find_bucket = typename meta::lift<find_bucket<N>::template type, typename mem::bucket_info<ID>::apply_block_size_seq::type>::type;

  };

  class StaticAllocator {
  public:
    template<size_t ID, typename T>
    static T* allocate() {
      static constexpr size_t N = sizeof(T);
      static constexpr size_t I = apply_find_bucket<ID, N>::value;
      return (T*)FWD(mem::get_pool<ID>().template get<I>().add_object());
    }
    
    template<size_t ID, typename T>
    static b8 deallocate(T* ptr) {
      static constexpr size_t N = sizeof(T);
      static constexpr size_t I = apply_find_bucket<ID, N>::value;
      if(mem::get_pool<ID>().template get<I>().remove_object(static_cast<void*>(FWD(ptr)))) 
        return true;
      return false;
    }

    template<size_t ID, typename T, typename U>
    static void* rebind(U* src) {
      auto new_ptr = allocate<ID, T>();
      L_ASSERT(new_ptr != nullptr);
      auto size = CLAMP_TOP(sizeof(T), sizeof(U));
      MEM_COPY(new_ptr, src, size);
      deallocate<ID, U>(src);
      return new_ptr;
    }

  private:
    template<size_t N>
    struct find_bucket {
      template<typename... Idxs>
      using type = typename meta::pred_find_t<meta::less_t<IdxT<N>>::template type>::template type<Idxs...>::type;
    };

    template<size_t ID, size_t N>
    using apply_find_bucket = typename meta::lift<find_bucket<N>::template type, typename mem::bucket_info<ID>::apply_block_size_seq::type>::type;

  };

  template<size_t ID>
  class RuntimeAllocator {
  public:
    static void* allocate(size_t size, size_t alignment = 0) {
      static constexpr auto size_array = index_array<typename mem::bucket_info<ID>::apply_block_size_seq::type>;
      size = MAX(size, alignment);
      auto func = [](size_t size) {
        for(size_t i = 0; i < mem::bucket_info<ID>::bucket_count; i++) {
          if(size <= size_array[i]){
            return i;
          }
        }
        return MAX_u64;
      };

      dispatcher _dispatcher{ IdxV<mem::bucket_info<ID>::bucket_count>
                            , [&]<size_t Index>(IdxT<Index>){
                              return (void*)mem::get_pool<ID>()
                                .template get<Index>()
                                .add_object();
                            }};

      return _dispatcher(FWD(func(size)));
    }

    static b8 free(void* ptr) {
      dispatcher _dispatcher{ IdxV<mem::bucket_info<ID>::bucket_count>
                            , [&]<size_t Index>(IdxT<Index>){
                              if(mem::get_pool<ID>().template get<Index>().belongs(ptr)) {
                                mem::get_pool<ID>().template get<Index>().remove_object(ptr);
                                return true;
                              }
                              return false;
                            }};
      for(size_t i = 0; i < mem::bucket_info<ID>::bucket_count; i++) {
        if(_dispatcher(i)) 
          return true;
      }
      return false;
    }

    static void* reallocate(void* ptr, size_t size, size_t alignment = DEFAULT_ALIGNMENT) {
      dispatcher _dispatcher{ IdxV<mem::bucket_info<ID>::bucket_count>
                            , [&]<size_t Index>(IdxT<Index>){
                              if(mem::get_pool<ID>().template get<Index>().belongs(ptr)) {
                                return mem::get_pool<ID>().template get<Index>().get_block_size();
                              }
                              return (size_t)0;
                            }};
      
      size = MAX(size, alignment);
      void* new_ptr = allocate(size);
      size_t old_size{};
      for(size_t i = 0; i < mem::bucket_info<ID>::bucket_count; i++) {
        old_size = _dispatcher(i);
        if(old_size)
          break;
      }
      L_ASSERT(new_ptr != nullptr);
      old_size = MIN(size, old_size);
      MEM_COPY(new_ptr, ptr, old_size);
      free(ptr);
      return new_ptr;
    } 


  };

  class LockFreeStaticAllocator {
  public:
    template<size_t ID, typename T>
    static T* allocate() {
      static constexpr size_t N = sizeof(T);
      static constexpr size_t I = apply_find_bucket<ID, N>::value;
      return (T*)FWD(mem::get_lock_free_pool<ID>().template get<I>().add_object());
    }
    
    template<size_t ID, typename T>
    static b8 deallocate(T* ptr) {
      static constexpr size_t N = sizeof(T);
      static constexpr size_t I = apply_find_bucket<ID, N>::value;
      if(mem::get_lock_free_pool<ID>().template get<I>().remove_object(static_cast<void*>(FWD(ptr)))) 
        return true;
      return false;
    }

    template<size_t ID, typename T, typename U>
    static void* rebind(U* src) {
      auto new_ptr = allocate<ID, T>();
      L_ASSERT(new_ptr != nullptr);
      auto size = CLAMP_TOP(sizeof(T), sizeof(U));
      MEM_COPY(new_ptr, src, size);
      deallocate<ID, U>(src);
      return new_ptr;
    }

  private:
    template<size_t N>
    struct find_bucket {
      template<typename... Idxs>
      using type = typename meta::pred_find_t<meta::less_t<IdxT<N>>::template type>::template type<Idxs...>::type;
    };

    template<size_t ID, size_t N>
    using apply_find_bucket = typename meta::lift<find_bucket<N>::template type, typename mem::bucket_info<ID>::apply_block_size_seq::type>::type;

  };

  template<size_t ID>
  class LockFreeRuntimeAllocator {
  public:
    static void* allocate(size_t size, size_t alignment = 0) {
      static constexpr auto size_array = index_array<typename mem::bucket_info<ID>::apply_block_size_seq::type>;
      size = MAX(size, alignment);
      auto func = [](size_t size) {
        for(size_t i = 0; i < mem::bucket_info<ID>::bucket_count; i++) {
          if(size <= size_array[i]){
            return i;
          }
        }
        return MAX_u64;
      };

      dispatcher _dispatcher{ IdxV<mem::bucket_info<ID>::bucket_count>
                            , [&]<size_t Index>(IdxT<Index>){
                              return (void*)mem::get_lock_free_pool<ID>()
                                .template get<Index>()
                                .get_object();
                            }};

      return _dispatcher(FWD(func(size)));
    }

    static b8 free(void* ptr) {
      dispatcher _dispatcher{ IdxV<mem::bucket_info<ID>::bucket_count>
                            , [&]<size_t Index>(IdxT<Index>){
                              if(mem::get_lock_free_pool<ID>().template get<Index>().belongs(ptr)) {
                                using BlockType = typename mem::LockFreeStaticMemoryPool<ID>::template type_at_index<Index>;
                                mem::get_lock_free_pool<ID>().template get<Index>().return_object(ptr);
                                return true;
                              }
                              return false;
                            }};
      for(size_t i = 0; i < mem::bucket_info<ID>::bucket_count; i++) {
        if(_dispatcher(i)) 
          return true;
      }
      return false;
    }

    static void* reallocate(void* ptr, size_t size, size_t alignment = DEFAULT_ALIGNMENT) {
      dispatcher _dispatcher{ IdxV<mem::bucket_info<ID>::bucket_count>
                            , [&]<size_t Index>(IdxT<Index>){
                              if(mem::get_lock_free_pool<ID>().template get<Index>().belongs(ptr)) {
                                return mem::get_lock_free_pool<ID>().template get<Index>().get_object_size();
                              }
                              return (size_t)0;
                            }};
      
      size = MAX(size, alignment);
      void* new_ptr = allocate(size);
      size_t old_size{};
      for(size_t i = 0; i < mem::bucket_info<ID>::bucket_count; i++) {
        old_size = _dispatcher(i);
        if(old_size)
          break;
      }
      L_ASSERT(new_ptr != nullptr);
      old_size = MIN(size, old_size);
      MEM_COPY(new_ptr, ptr, old_size);
      free(ptr);
      return new_ptr;
    } 

  };

  namespace mem {
    using default_bucket_descriptor_set_t
          = List
            < defaults::Bucket_8<16384>
            , defaults::Bucket_64<16384>
            , defaults::Bucket_256<16384>
            , defaults::Bucket_1024<4096>
            , defaults::Bucket_16384<1024>
            , defaults::Bucket_256KB<128>
            , defaults::Bucket_1M<64>
            , defaults::Bucket_16M<16>>;

#ifndef LOFI_DEFAULT_BUCKETS_COUNT
#define LOFI_DEFAULT_BUCKETS_COUNT 4
#endif
// if using custom buckets
#if (LOFI_DEFAULT_BUCKETS_COUNT == 1)

      template<>
      struct BucketDescriptorSet<0> {
        using type = default_bucket_descriptor_set_t;
      };

#elif (LOFI_DEFAULT_BUCKETS_COUNT == 2)
      
      template<>
      struct BucketDescriptorSet<0> {
        using type = default_bucket_descriptor_set_t;
      };

      template<>
      struct BucketDescriptorSet<1> {
        using type = default_bucket_descriptor_set_t;
      };
   
#elif (LOFI_DEFAULT_BUCKETS_COUNT == 3)
 
      template<>
      struct BucketDescriptorSet<0> {
        using type = default_bucket_descriptor_set_t;
      };

      template<>
      struct BucketDescriptorSet<1> {
        using type = default_bucket_descriptor_set_t;
      };
   
      template<>
      struct BucketDescriptorSet<2> {
        using type = default_bucket_descriptor_set_t;
      };
 
#elif (LOFI_DEFAULT_BUCKETS_COUNT == 4)

      template<>
      struct BucketDescriptorSet<0> {
        using type = default_bucket_descriptor_set_t;
      };

      template<>
      struct BucketDescriptorSet<1> {
        using type = default_bucket_descriptor_set_t;
      };
   
      template<>
      struct BucketDescriptorSet<2> {
        using type = default_bucket_descriptor_set_t;
      };
 
      template<>
      struct BucketDescriptorSet<3> {
        using type = default_bucket_descriptor_set_t;
      };
 
#else

      template<>
      struct BucketDescriptorSet<0> {
        using type = default_bucket_descriptor_set_t;
      };

      template<>
      struct BucketDescriptorSet<1> {
        using type = default_bucket_descriptor_set_t;
      };
   
      template<>
      struct BucketDescriptorSet<2> {
        using type = default_bucket_descriptor_set_t;
      };
 
      template<>
      struct BucketDescriptorSet<3> {
        using type = default_bucket_descriptor_set_t;
      };
 
      template<>
      struct BucketDescriptorSet<4> {
        using type = default_bucket_descriptor_set_t;
      };

//#else
//      template<>
//      struct BucketDescriptorSet<0> {
//        using type = default_bucket_descriptor_set_t;
//      };

#endif
  }		// -----  end of namespace mem  ----- 

}		// -----  end of namespace lofi  ----- 
