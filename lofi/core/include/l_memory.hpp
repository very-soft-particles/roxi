// =====================================================================================
//
//       Filename:  MemoryPool.hpp
//
//    Description: 
//
//
//        Version:  1.0
//        Created:  2023-03-15 9:55:45 AM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Roxi Graves (rg), nada
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#pragma once

// my includes

#include "l_base.hpp"
#include "l_vocab.hpp"
#include "l_string.hpp"
#include "l_tuple.hpp"

#include "l_sync.hpp"

#define DEFAULT_ALIGNMENT 64

namespace lofi {

  template<size_t Size>
  struct choose_index_type {
    using type = typename meta::index_type_table<meta::get_type_table_index<Size>>::type;
  };

  template<typename T>
  struct index_type_max;

  template<>
  struct index_type_max<u8> : Value<u8, MAX_u8> {};

  template<>
  struct index_type_max<u16> : Value<u16, MAX_u16> {};

  template<>
  struct index_type_max<u32> : Value<u32, MAX_u32> {};

  template<>
  struct index_type_max<u64> : Value<u64, MAX_u64> {};

  template<class Size>
  struct round_to_next_pow_2;

  template<u8 I>
  struct round_to_next_pow_2<Value<u8, I>>
  : Value<u8, (((I - 1) | 1 | 2 | 4) + 1)> {};

  template<u16 I>
  struct round_to_next_pow_2<Value<u16, I>>
  : Value<u8, (((I - 1) | 1 | 2 | 4 | 8) + 1)> {};

  template<u32 I>
  struct round_to_next_pow_2<Value<u32, I>>
  : Value<u8, (((I - 1) | 1 | 2 | 4 | 8 | 16) + 1)> {};

  template<u64 I>
  struct round_to_next_pow_2<Value<u64, I>>
  : Value<u8, (((I - 1) | 1 | 2 | 4 | 8 | 16 | 32) + 1)> {};

  namespace mem {

    namespace helpers {

//      static const b8 is_allocated();
//
//      static void* get_tls(size_t thread_id);
//
//      b8 allocate_tls(size_t num_threads, size_t n); 
//        
//      void deallocate_tls(size_t num_threads); 
                
      static inline void* runtime_align_ptr(const void* ptr, const u64 align) {
        L_ASSERT((align & (align - 1)) == 0 && "ptr alignment failure... alignment is not a power of 2");
        auto new_ptr = PTR2INT((u8*)ptr);
        return INT2PTR(ALIGN_POW2(PTR2INT(ptr), align));
      }
              
      template<size_t Align>
      static inline void* align_ptr(const void* ptr) {
        static_assert((Align & (Align - 1)) == 0, "ptr alignment failure... alignment is not a power of 2");
        auto new_ptr = PTR2INT((u8*)ptr);
        return INT2PTR(ALIGN_POW2(PTR2INT(ptr), Align));
      }

      template<class T>
      static constexpr b8 is_size_type = List<u8, u16, u32, u64, size_t>::template has<T>::value;

      template<typename SizeT>
      static inline u64 compile_rand(SizeT hash_type = 0) {
#define TIME0 (__TIME__[0])
#define TIME1 (__TIME__[1])
#define TIME2 (__TIME__[2])
#define TIME3 (__TIME__[3])
#define TIME4 (__TIME__[5])
#define TIME5 (__TIME__[7])
        static_assert(is_size_type<SizeT>, "used incorrect SizeT in compile_rand(HashType), must be size type");
      
     		constexpr unsigned int BIT_NOISE1 = 0xB5297A4D;
     		constexpr unsigned int BIT_NOISE2 = 0x68E31DA4;
     		constexpr unsigned int BIT_NOISE3 = 0x1B56C4E9;
        uint32_t val = TIME0;
        val += TIME1;
        val += TIME2;
        val += TIME3;
        val += TIME4;
        val += TIME5;
     		unsigned int mangled = val;
     		mangled *= BIT_NOISE1;
     		// add seed here, or partway through the func
     		mangled ^= (mangled >> 8);
     		mangled += BIT_NOISE2;
    		mangled ^= (mangled << 8);
      	mangled *= BIT_NOISE3;
      	mangled ^= (mangled >> 8);
        return mangled;
      #undef TIME0
      #undef TIME1
      #undef TIME2
      #undef TIME3
      #undef TIME4
      #undef TIME5
      }
      
      template<class SizeT>
      static const SizeT runtime_rand(SizeT val = 0) {
        static_assert(is_size_type<SizeT>, "used incorrect SizeT in runtime_hash(SizeT), must be size type");
     		constexpr unsigned int BIT_NOISE1 = 0xB5297A4D;
     		constexpr unsigned int BIT_NOISE2 = 0x68E31DA4;
     		constexpr unsigned int BIT_NOISE3 = 0x1B56C4E9;
     		unsigned int mangled = val;
        mangled += time(NULL);
     		mangled *= BIT_NOISE1;
     		// add seed here, or partway through the func
     		mangled ^= (mangled >> 8);
     		mangled += BIT_NOISE2;
    		mangled ^= (mangled << 8);
      	mangled *= BIT_NOISE3;
      	mangled ^= (mangled >> 8);
        return mangled;
      }


      template<class SizeT>
      class Hasher {
      public:
        Hasher() {}
        static inline u64 hash(SizeT val = 0) {
          constexpr unsigned int BIT_NOISE1 = 0xB5297A4D;
     		  constexpr unsigned int BIT_NOISE2 = 0x68E31DA4;
     		  constexpr unsigned int BIT_NOISE3 = 0x1B56C4E9;
      
     		  unsigned int mangled = val;
     		  mangled *= BIT_NOISE1;
     		  // add seed here, or partway through the func
     		  mangled ^= (mangled >> 8);
     		  mangled += BIT_NOISE2;
    		  mangled ^= (mangled << 8);
      	  mangled *= BIT_NOISE3;
      	  mangled ^= (mangled >> 8);
          return mangled;
        };
      private:
      };

      template<>
      class Hasher<const char*> {
      public:
        Hasher() {}
        static inline u64 hash(const char* val) {
          constexpr unsigned int BIT_NOISE1 = 0xB5297A4D;
     		  constexpr unsigned int BIT_NOISE2 = 0x68E31DA4;
     		  constexpr unsigned int BIT_NOISE3 = 0x1B56C4E9;
     		  unsigned int mangled = 0;

          while(*val != '\0') {
            mangled += *val++;
          }
      
     		  mangled *= BIT_NOISE1;
     		  // add seed here, or partway through the func
     		  mangled ^= (mangled >> 8);
     		  mangled += BIT_NOISE2;
    		  mangled ^= (mangled << 8);
      	  mangled *= BIT_NOISE3;
      	  mangled ^= (mangled >> 8);
          return mangled;
        };
      private:
      };

      template<>
      class Hasher<String> {
      public:
        Hasher() {}
        static inline u64 hash(String val) {
          constexpr unsigned int BIT_NOISE1 = 0xB5297A4D;
     		  constexpr unsigned int BIT_NOISE2 = 0x68E31DA4;
     		  constexpr unsigned int BIT_NOISE3 = 0x1B56C4E9;
     		  unsigned int mangled = 0;

          u64 i = 0;
          while(val.str[i] != '\0') {
            mangled += val.str[i++];
          }
      
     		  mangled *= BIT_NOISE1;
     		  // add seed here, or partway through the func
     		  mangled ^= (mangled >> 8);
     		  mangled += BIT_NOISE2;
    		  mangled ^= (mangled << 8);
      	  mangled *= BIT_NOISE3;
      	  mangled ^= (mangled >> 8);
          return mangled;
        };
      private:
      };


    }		// -----  end of namespace helpers  ----- 

//    static b8 allocate_thread_storage(size_t num_threads, size_t size) {
//      if(helpers::allocate_tls(num_threads, size))
//        return true;
//      return false;
//    }
    
//    template<size_t ThreadID>
//    class TLSAllocPolicy {
//    public:
//      static void* allocate();
//      static void deallocate();
//      static void* data();
//    private:
//      static void* do_align(void* ptr);
//      static void* do_unalign(void* ptr);
//    };
   
    template<size_t N, size_t Align = 64>
    class MAllocPolicy {
    public:
      using index_t = typename choose_index_type<N>::type;
      ~MAllocPolicy();
      static constexpr size_t get_size();
      void* allocate();
      void* move(const void* ptr);
      void deallocate();
      void* data() const;
      bool belongs(void* ptr);
    private:
      void* data_ptr = nullptr;
      static void* do_align(const void* ptr);
      static void* do_unalign(const void* ptr);
    };

    template<size_t N, size_t Align = 64>
    class StackAllocPolicy {
    public:
      using index_t = typename choose_index_type<N>::type;
      using byte_t = uint8_t;
      static constexpr size_t get_size();
      void* allocate();
      void deallocate();
      void* data() const;
      bool belongs(void* ptr);
    private:
      byte_t data_ptr[N + Align];
    };


    template<size_t N, size_t Align = 64>
    class SubAllocPolicy {
    public:
      using index_t = typename choose_index_type<N>::type;
      SubAllocPolicy() {}
      SubAllocPolicy(void* ptr);
      static constexpr size_t get_size();
      void* allocate();
      void* allocate(void* ptr);
      void* move(const void* ptr);
      void deallocate();
      void* data() const;
      bool belongs(void* ptr);
    private:
      void* data_ptr = nullptr;
    };

    template<size_t N, size_t Align>
    MAllocPolicy<N, Align>::~MAllocPolicy(){
      deallocate();
    }

    template<size_t N, size_t Align>
    void MAllocPolicy<N, Align>::deallocate(){
      if(data_ptr) {
        data_ptr = (void*)do_unalign(data_ptr);
        free(data_ptr);
      }
      data_ptr = nullptr;
    }

    template<size_t N, size_t Align>
    void* MAllocPolicy<N, Align>::allocate() {
        //PRINT_S("entering MAlloc allocate in malloc\n");
      if(!data_ptr) {
        //PRINT_S("allocating in malloc\n");
        data_ptr = malloc(N + Align);
        if(data_ptr) {
          //PRINT_S("successfully allocated\n");
          data_ptr = do_align(data_ptr);
          return data_ptr;
        }
        //PRINT_S("failed to allocate in MAllocPolicy\n");
        return nullptr;
      }
      return data_ptr;
    }

    template<size_t N, size_t Align>
    void* MAllocPolicy<N, Align>::move(const void* ptr) {
      data_ptr = do_align(ptr);
      return data_ptr;
    }

    template<size_t N, size_t Align>
    void* MAllocPolicy<N, Align>::data() const {
      return data_ptr;
    }

    template<size_t N, size_t Align>
    constexpr size_t MAllocPolicy<N, Align>::get_size() {
      return N;
    }

    template<size_t N, size_t Align>
    bool MAllocPolicy<N, Align>::belongs(void* ptr) {
      if(ptr >= data() && ptr < (uint8_t*)data() + N + Align)
        return true;
      return false;
    }

    template<size_t N, size_t Align>
    void* MAllocPolicy<N, Align>::do_align(const void* ptr) {
      u8* align_ptr = (u8*)helpers::align_ptr<Align>(ptr);
      u64 ptr_diff = PTR2INT(align_ptr - (u8*)ptr);
      L_ASSERT(ptr_diff < 256);
      if(!ptr_diff) {
        align_ptr += Align;
      }
      u8* new_ptr = align_ptr - 1;
      *new_ptr = (u8)ptr_diff;
      return (void*)align_ptr;
    }

    template<size_t N, size_t Align>
    void* MAllocPolicy<N, Align>::do_unalign(const void* ptr) {
      u8* ptr_cast = (u8*)ptr;
      u8* new_ptr = ptr_cast - 1;
      u8 ptr_diff = (*new_ptr);
      return (void*)(ptr_cast - ptr_diff);
    }

//    template<size_t ThreadID>
//    void* TLSAllocPolicy<ThreadID>::allocate() {
//      if(helpers::is_allocated()) [[likely]] {
//        return helpers::get_tls(ThreadID);
//      }
//      return nullptr;
//    }
//
//    template<size_t ThreadID>
//    void TLSAllocPolicy<ThreadID>::deallocate(){
//      if(helpers::get_tls(ThreadID) != nullptr)
//        helpers::deallocate_tls(ThreadID);
//    }
//
//
//    template<size_t ThreadID>
//    void* TLSAllocPolicy<ThreadID>::data() {
//      return helpers::get_tls(ThreadID);
//    }

    template<size_t N, size_t Align>
    void* StackAllocPolicy<N, Align>::allocate() {
      return helpers::align_ptr<Align>(data_ptr);
    }

    template<size_t N, size_t Align>
    void StackAllocPolicy<N, Align>::deallocate() {}

    template<size_t N, size_t Align>
    constexpr size_t StackAllocPolicy<N, Align>::get_size() {
      return N;
    }

    template<size_t N, size_t Align>
    void* StackAllocPolicy<N, Align>::data() const {
      return helpers::align_ptr<Align>((void*)data_ptr);
    }

    template<size_t N, size_t Align>
    bool StackAllocPolicy<N, Align>::belongs(void* ptr) {
      if(ptr >= &data_ptr[0] && ptr < &data_ptr[N + Align]) {
        return true;
      }
      return false;
    }

    template<size_t N, size_t Align>
    SubAllocPolicy<N, Align>::SubAllocPolicy(void* ptr) 
      : data_ptr(helpers::align_ptr<Align>(ptr)) {}

    template<size_t N, size_t Align>
    void* SubAllocPolicy<N, Align>::allocate() {
      return data_ptr;
    }

    template<size_t N, size_t Align>
    void* SubAllocPolicy<N, Align>::allocate(void* ptr) {
      if(!data_ptr)
        data_ptr = helpers::align_ptr<Align>((void*)ptr);
      return data_ptr;
    }

    template<size_t N, size_t Align>
    void* SubAllocPolicy<N, Align>::move(const void* ptr) {
      data_ptr = helpers::align_ptr<Align>((void*)ptr);
      return data_ptr;
    }


    template<size_t N, size_t Align>
    void SubAllocPolicy<N, Align>::deallocate() {}

    template<size_t N, size_t Align>
    constexpr size_t SubAllocPolicy<N, Align>::get_size() {
      return N;
    }

    template<size_t N, size_t Align>
    void* SubAllocPolicy<N, Align>::data() const {
      return data_ptr;
    }

    template<size_t N, size_t Align>
    bool SubAllocPolicy<N, Align>::belongs(void* ptr) {
      if(!data_ptr)
        return false;
      if(ptr >= data() && ptr < (uint8_t*)data() + N)
        return true;
      return false;
    }
  }

}
