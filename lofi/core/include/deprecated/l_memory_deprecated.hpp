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

#include "l_base.h"
#include "l_vocab.hpp"

#define ALIGNMENT 64

namespace lofi {

  template<size_t Size>
  struct choose_index_type {
    using type = typename meta::index_type_table<meta::get_type_table_index<Size>>::type;
  };

  namespace mem {

    namespace helpers {
        template<size_t Align>
        static inline void* align_ptr(void* ptr) {
          static_assert((Align & (Align - 1)) == 0, "ptr alignment failure... alignment is not a power of 2");
          auto new_ptr = PTR2INT((u8*)ptr);
          return INT2PTR(ALIGN_POW2(PTR2INT(ptr), Align));
        }
    }		// -----  end of namespace helpers  ----- 

    template<size_t N, size_t Align = 64>
    class MAllocPolicy {
    public:
      using index_t = typename choose_index_type<N>::type;
      ~MAllocPolicy();
      static constexpr size_t get_size();
      void* allocate();
      void* move(void* ptr);
      void deallocate();
      void* data() const;
      bool belongs(void* ptr);
    private:
      void* data_ptr = nullptr;
      static void* do_align(void* ptr);
      static void* do_unalign(void* ptr);
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
      SubAllocPolicy() = delete;
      SubAllocPolicy(void* ptr);
      static constexpr size_t get_size();
      void* allocate();
      void* allocate(void* ptr);
      void* move(void* ptr);
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
      if(!data_ptr) {
        data_ptr = malloc(N + Align);
        data_ptr = do_align(data_ptr);
      }
      return data_ptr;
    }

    template<size_t N, size_t Align>
    void* MAllocPolicy<N, Align>::move(void* ptr) {
      data_ptr = do_align(ptr);
      return data_ptr;
    }

    template<size_t N, size_t Align>
    void* MAllocPolicy<N, Align>::data() const {
      return (data_ptr);
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
    void* MAllocPolicy<N, Align>::do_align(void* ptr) {
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
    void* MAllocPolicy<N, Align>::do_unalign(void* ptr) {
      u8* ptr_cast = (u8*)ptr;
      u8* new_ptr = ptr_cast - 1;
      u8 ptr_diff = (*new_ptr);
      return (void*)(ptr_cast - ptr_diff);
    }

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
    void* SubAllocPolicy<N, Align>::move(void* ptr) {
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

    template<class T, size_t Size, size_t Alignment, template<size_t, size_t> class AllocPolicy>
    class PackedArrayContainerPolicy : AllocPolicy<Size * sizeof(T), Alignment> {
    public:
      using alloc_t = AllocPolicy<Size * sizeof(T), Alignment>;
      using alloc_t::data;
      using index_t = typename alloc_t::index_t;
      using alloc_ptr_t = typename alloc_t::ptr_t;
      using value_t = T;
      using ptr_t = T*;
      using ref_t = T&;
  
      PackedArrayContainerPolicy() {
        alloc_t::allocate();
      }

      ~PackedArrayContainerPolicy() {}
  
      T& operator[](const index_t index) {
        return static_cast<T*>(alloc_t::data())[index];
      }
      
      index_t add_object(T&& obj) {
        L_ASSERT(top != Size);
        static_cast<T*>(alloc_t::data())[top] = obj;
        return top++;
      }
  
      void remove_object(index_t index) {
        static_cast<T*>(alloc_t::data())[index] = static_cast<T*>(alloc_t::data())[--top];
      }
  
      void clear() {
        top = 0;
      }
  
    private:
      index_t top = 0;
    };

    template<class T, size_t Size, size_t Alignment>
    class PackedArrayContainerPolicy<T, Size, Alignment, SubAllocPolicy> : SubAllocPolicy<Size * sizeof(T), Alignment> {
    public:
      using alloc_t = SubAllocPolicy<Size * sizeof(T), Alignment>;
      using index_t = typename alloc_t::index_t;
      using value_t = T;
      using ptr_t = T*;
      using ref_t = T&;
 
      PackedArrayContainerPolicy() = delete;
      PackedArrayContainerPolicy(void* ptr) : alloc_t(ptr) {}

      ~PackedArrayContainerPolicy() {}
  
      T& operator[](const index_t index) {
        return static_cast<T*>(alloc_t::data())[index];
      }
      
      size_t add_object(T&& obj) {
        L_ASSERT(top != Size);
        static_cast<T*>(alloc_t::data())[top] = obj;
        return top++;
      }
  
      void remove_object(size_t index) {
        static_cast<T*>(alloc_t::data())[index] = static_cast<T*>(alloc_t::data())[--top];
      }
  
      void clear() {
        top = 0;
      }
  
    private:
      index_t top = 0;
    };

    template<class T, size_t Size, size_t Alignment, template<size_t, size_t> class AllocPolicy = SubAllocPolicy>
    class SparseArrayContainerPolicy : AllocPolicy<Size * sizeof(T), Alignment> {
    public:
      using alloc_t = AllocPolicy<Size * sizeof(T), Alignment>;
      using alloc_t::data;
      using index_t = typename alloc_t::index_t;
      using value_t = T;
      using ptr_t = T*;
      using ref_t = T&;
  
      SparseArrayContainerPolicy() {
        static_assert(sizeof(T) >= sizeof(uint32_t), "Size too small for T in SparseArray");
        alloc_t::allocate(sizeof(T) * Size);
        reset_indices();
      }
  
      ~SparseArrayContainerPolicy() {}

      T& operator[](const index_t index) {
        return static_cast<T*>(alloc_t::data())[index];
      }
 
      index_t add_object(T obj) {
        L_ASSERT(top != UINT32_MAX);
        auto here = *get_handle(top);
        auto temp = *get_handle(here);
  
        static_cast<T*>(alloc_t::data())[top] = obj;
  
        here = top;
        top = temp;
  
        return here;
      }
  
      void remove_object(const index_t index) {
        *get_handle(index) = top;
        top = index;
      }
  
      static constexpr size_t get_size() {
        return Size;
      }
  
    private:
      void reset_indices() {
        for(index_t i = 0; i < Size; i++) {
          if(i + 1 == Size) {
            *get_handle(i) = UINT32_MAX;
            break;
          }
          *get_handle(i) = i + 1;
        }
      }
  
      inline index_t* get_handle(const index_t index) {
        return ((uint32_t*)(static_cast<T*>(alloc_t::data()) + index));
      }
  
      index_t top = 0;
    };
  

    template<class T, size_t Size, size_t Alignment>
    class SparseArrayContainerPolicy<T, Size, Alignment, SubAllocPolicy> : SubAllocPolicy<Size * sizeof(T), Alignment> {
    public:
      using alloc_t = SubAllocPolicy<Size * sizeof(T), Alignment>;
      using alloc_t::data;
      using value_t = T;
      using ptr_t = T*;
      using ref_t = T&;
      using index_t = typename alloc_t::index_t;


      SparseArrayContainerPolicy() = delete;
  
      SparseArrayContainerPolicy(void* ptr) : alloc_t(ptr) {
        static_assert(sizeof(T) >= sizeof(index_t), "size of T too small in SparseArray");
        reset_indices();
      }
  
      ~SparseArrayContainerPolicy() {}
 
      T& operator[](const index_t index) {
        return static_cast<T*>(alloc_t::data())[index];
      }
  
      index_t add_object(T obj) {
  
        L_ASSERT(top != UINT32_MAX);
        auto here = *get_handle(top);
        auto temp = *get_handle(here);
  
        static_cast<T*>(alloc_t::data())[top] = obj;
  
        here = top;
        top = temp;
  
        return here;
      }
  
      void remove_object(const index_t index) {
        *get_handle(index) = top;
        top = index;
      }
  
      static constexpr size_t get_size() {
        return Size;
      }
  
    private:
      void reset_indices() {
        for(size_t i = 0; i < Size; i++) {
          if(i + 1 == Size) {
            *get_handle(i) = UINT32_MAX;
            break;
          }
          *get_handle(i) = i + 1;
        }
      }
  
      inline index_t* get_handle(const index_t index) {
        return ((uint32_t*)(static_cast<T*>(alloc_t::data()) + index));
      }
  
      uint32_t top = 0;
    };
   

    template<class KeyType>
    struct choose_null_key;

    template<>
    struct choose_null_key<uint8_t> {
      static constexpr uint8_t value = MAX_u8;
    };

    template<>
    struct choose_null_key<uint16_t> {
      static constexpr uint16_t value = MAX_u16;
    };

    template<>
    struct choose_null_key<uint32_t> {
      static constexpr uint32_t value = MAX_u32;
    };

    template<>
    struct choose_null_key<uint64_t> {
      static constexpr uint64_t value = MAX_u64;
    };

    template<class T, size_t Alignment, template<size_t, size_t> class AllocPolicy, class KeyType = uint32_t, uint32_t Size = 64>
    class Map : AllocPolicy<(sizeof(T) + sizeof(KeyType)) * Size, Alignment> {
    public:
      using alloc_t = AllocPolicy<(sizeof(T) + sizeof(KeyType)) * Size, Alignment>;
      static constexpr KeyType null_key = choose_null_key<KeyType>::value;

      
      Map() {
        static_assert(((Size - 1) & (~Size)) == (Size - 1), "Size of map is not a power of two");
        reset_keys();
      }
  
      ~Map() {
      }
  
      T& operator[](const KeyType index) {
        const KeyType true_key = find_key(index);
        L_ASSERT(true_key != null_key);
        return get_values(true_key);
      }
  
      KeyType insert(const uint32_t key, const T t) {
        auto new_key = get_new_key(key);
        if(new_key == null_key)
          return null_key;
        get_keys()[new_key] = key;
        get_values()[new_key] = t;
      }
  
      void remove(const KeyType key) {
        auto new_key = find_key(key);
        set_key_to_null(new_key);
      }
  
      bool has(const uint32_t key) {
        return find_key(key) != null_key;
      }
  
    private:
      void reset_keys() {
        for(size_t i = 0; i < Size; i++) {
          set_key_to_null(i);
        }
      }
  
      KeyType find_key(const KeyType key) {
        uint32_t acc = key & (Size - 1);
        auto count = Size;
  
        while(count--) {
          if(get_keys()[acc] == key)
            return acc;
          ++acc;
          acc &= (Size - 1);
        }
  
        return null_key;
      }
  
      KeyType get_new_key(const KeyType key) {
        KeyType acc = key & (Size - 1);
        auto count = Size;
  
        while(count--) {
          if(key_is_null(acc))
            return acc;
          ++acc;
          acc &= (Size - 1);
        }
  
        return null_key;
      }
  
      inline bool key_is_null(const KeyType key) {
        return get_keys()[key] == null_key;
      }
  
      inline void set_key_to_null(const KeyType key) {
        get_keys()[key] = null_key;
      }
  
      inline KeyType* get_keys() {
        return static_cast<KeyType*>(alloc_t::data());
      }
  
      inline T* get_values() {
        return static_cast<T*>(get_keys() + Size);
      }
  
    };



    template<class T, size_t Alignment, class KeyType, uint32_t Size>
    class Map<T, Alignment, SubAllocPolicy, KeyType, Size> : SubAllocPolicy<(sizeof(T) + sizeof(KeyType)) * Size> {
    public:
      using alloc_t = SubAllocPolicy<(sizeof(T) + sizeof(KeyType)) * Size>;
      static constexpr KeyType null_key = choose_null_key<KeyType>::value;

      Map() = delete;

      Map(void* ptr) : alloc_t(ptr) {
        static_assert(((Size - 1) & (~Size)) == (Size - 1), "Size of map is not a power of two");
        reset_keys();
      }
  
      ~Map() {
      }
  
      T& operator[](const KeyType index) {
        const KeyType true_key = find_key(index);
        L_ASSERT(true_key != null_key);
        return get_values(true_key);
      }
  
      uint32_t insert(const uint32_t key, const T t) {
        auto new_key = get_new_key(key);
        if(new_key == UINT32_MAX)
          return UINT32_MAX;
        get_keys()[new_key] = key;
        get_values()[new_key] = t;
      }
  
      void remove(const KeyType key) {
        auto new_key = find_key(key);
        set_key_to_null(new_key);
      }
  
      bool has(const uint32_t key) {
        return find_key(key) != UINT32_MAX;
      }
  
    private:
      void reset_keys() {
        for(size_t i = 0; i < Size; i++) {
          set_key_to_null(i);
        }
      }
  
      KeyType find_key(const KeyType key) {
        uint32_t acc = key & (Size - 1);
        auto count = Size;
  
        while(count--) {
          if(get_keys()[acc] == key)
            return acc;
          ++acc;
          acc &= (Size - 1);
        }
  
        return UINT32_MAX;
      }
  
      KeyType get_new_key(const KeyType key) {
        KeyType acc = key & (Size - 1);
        auto count = Size;
  
        while(count--) {
          if(key_is_null(acc))
            return acc;
          ++acc;
          acc &= (Size - 1);
        }
  
        return UINT32_MAX;
      }
  
      inline bool key_is_null(const KeyType key) {
        return get_keys()[key] == UINT32_MAX;
      }
  
      inline void set_key_to_null(const KeyType key) {
        get_keys()[key] = UINT32_MAX;
      }
  
      inline KeyType* get_keys() {
        return static_cast<KeyType*>(alloc_t::data());
      }
  
      inline T* get_values() {
        return static_cast<T*>(get_keys() + Size);
      }
  
    };



    template<class T, size_t N, size_t Align, template<size_t, size_t> class AllocPolicy = SubAllocPolicy>
    class FreeListContainerPolicy : private AllocPolicy<N * sizeof(T), Align> {
    public:
      using alloc_t = AllocPolicy<N * sizeof(T), Align>;
      using alloc_t::data;
      using value_t = T;
      using ptr_t = T*;
      using ref_t = T&;
      
      FreeListContainerPolicy() {
        reset_elements();
      }


      ~FreeListContainerPolicy(){};

      ptr_t add_object() {
        auto thisone = (T**)top;
        if(thisone == nullptr) {
          return nullptr;
        }

        auto temp = *thisone;
        auto ret = top;

        top = temp;

        return (T*)ret;
      }

      bool remove_object(void* rhs) {
        if(!alloc_t::belongs((void*)rhs))
          return false;
        ptr_t temp = top;
        top = rhs;
        *((T**)top) = temp;
        return true;
      }

      T* clear() {
        reset_elements();
        return top;
      }

      static constexpr size_t get_block_size() {
        return sizeof(T);
      }

      static constexpr size_t get_block_count() {
        return N;
      }

    private:
      void reset_elements() {
        top = (ptr_t)alloc_t::data();

        for(size_t i = 0; i < N; i++)
        {
          size_t next = i + 1;
          ptr_t* temp = (T**)(top + i);
          if(next == N) {
            *temp = nullptr;
            break;
          }
          *temp = (top + next);
        }
      }

      T* top;
    
    };

    template<class T, size_t N, size_t Align>
    class FreeListContainerPolicy<T, N, Align, SubAllocPolicy> : private SubAllocPolicy<N * sizeof(T), Align> {
    public:
      using alloc_t = SubAllocPolicy<N * sizeof(T), Align>;
      using alloc_t::belongs;
      using value_t = T;
      using ptr_t = T*;
      using ref_t = T&;
      using alloc_t::data;
      FreeListContainerPolicy() = default;
      FreeListContainerPolicy(void* ptr);


      ~FreeListContainerPolicy() = default;
      void move_ptr(void* ptr) {
        top = this->allocate(ptr);
      }

      ptr_t add_object() {
        auto thisone = (T**)top;
        if(thisone == nullptr) {
          return nullptr;
        }

        auto temp = *thisone;
        auto ret = top;

        top = temp;

        return (T*)ret;
      }

      bool remove_object(void* rhs) {
        if(!alloc_t::belongs(rhs)){
          return false;
        }
        void** temp = (void**)rhs;
        *temp = top;
        top = (ptr_t)rhs;
        return true;
      };

      static constexpr size_t get_block_size() {
        return sizeof(T);
      }

      static constexpr size_t get_block_count() {
        return N;
      }

    private:
      void reset_elements() {
        top = (ptr_t)alloc_t::data();
        for(size_t i = 0; i < N; i++)
        {
          size_t next = i + 1;
          ptr_t* temp = (T**)(top + i);
          if(next == N) {
            *temp = nullptr;
            break;
          }
          *temp = (top + next);
        }
      }
      ptr_t top = nullptr;
    };


    template<size_t N, size_t Align, template<size_t, size_t> class AllocPolicy>
    class StackContainerPolicy : AllocPolicy<N, Align> {
    public:
      using byte_t = uint8_t;
      using alloc_t = AllocPolicy<N, Align>;
      using value_t = byte_t;
      using ptr_t = byte_t*;
      using ref_t = byte_t&;
      using alloc_t::belongs;

      StackContainerPolicy() {
        top = (ptr_t)alloc_t::allocate();
      };

      ptr_t push(size_t size) {
        if(!alloc_t::belongs((typename alloc_t::ptr_t)((uint8_t*)top + size)))
          return nullptr;

        auto ret = top;
        top += size;
        top = helpers::align_ptr<Align>( top);
        return ret;
      };

      void pop_to(ptr_t ptr) {
        top = ptr;
      };

      void clear() {
      }

    private:
      ptr_t top = nullptr;

    };


    template<size_t N, size_t Align>
    class StackContainerPolicy<N, Align, SubAllocPolicy> : SubAllocPolicy<N, Align> {
    public:
      using byte_t = uint8_t;
      using alloc_t = SubAllocPolicy<N>;
      using value_t = byte_t;
      using ptr_t = byte_t*;
      using ref_t = byte_t&;

      StackContainerPolicy(ptr_t ptr);

      ptr_t push(size_t size) {
        if(!alloc_t::belongs((typename alloc_t::ptr_t)((uint8_t*)top + size)))
          return nullptr;

        auto ret = top;
        top += size;
        top = helpers::align_ptr<Align>( top);
        return ret;
      };

      void pop_to(ptr_t ptr) {
        top = ptr;
      };

    private:
      ptr_t top = nullptr;

    };

    template<class T, size_t N, size_t Align, template<size_t, size_t> class AllocPolicy>
    class ArrayContainerPolicy : AllocPolicy<N * sizeof(T), Align> {
    public:
      using value_t = T;
      using ptr_t = T*;
      using ref_t = T&;
      using index_t = typename choose_index_type<N>::type;
      using alloc_t = AllocPolicy<N * sizeof(T), Align>;

      ArrayContainerPolicy() : alloc_t() {
        alloc_t::allocate();
      }
      
      ref_t operator[](index_t index) {
        return get_array_cast()[index];
      }

      const index_t get_size() const {
        return top;
      }

      ptr_t get_buffer() {
        return get_array_cast();
      }

      ptr_t push(index_t count) {
        index_t result = top;
        index_t new_top = top + count;
        if(new_top > N) {
          return nullptr;
        }
        top = new_top;
        return get_array_cast() + result;
      }

      static constexpr index_t get_cap() {
        return (index_t)N;
      }

      void pop(index_t count) {
        index_t new_top = top - count;
        if(new_top >= top){
          return;
        }
        top = new_top;
      }

    private:
      T* get_array_cast() {
        return (ptr_t)alloc_t::data();
      }
      index_t top = 0;
    };

    template<class T, size_t N, size_t Align>
    class ArrayContainerPolicy<T, N, Align, SubAllocPolicy> : SubAllocPolicy<N * sizeof(T), Align> {
    public:
      using value_t = T;
      using ptr_t = T*;
      using ref_t = T&;
      using const_ref_t = const T&;
      using index_t = typename choose_index_type<N>::type;
      using alloc_t = SubAllocPolicy<N * sizeof(T), Align>;

      ArrayContainerPolicy(void* ptr) : alloc_t((ptr_t)ptr) {}
      
      T& operator[](size_t index) {
        return get_array_cast()[index];
      }
 
      const index_t get_size() const {
        return top;
      }

      ptr_t get_buffer() {
        return get_array_cast();
      }

      ptr_t push(index_t count) {
        index_t result = top;
        index_t new_top = top + count;
        if(new_top > N) {
          return nullptr;
        }
        top = new_top;
        return get_array_cast() + result;
      }

      static constexpr index_t get_cap() {
        return (index_t)N;
      }

      void pop(index_t count) {
        index_t new_top = top - count;
        if(new_top >= top){
          return;
        }
        top = new_top;
      }

    private:
      T* get_array_cast() {
        return (ptr_t)alloc_t::data();
      }
      index_t top = 0;
    };


    template<class T, size_t N, size_t Align>
    FreeListContainerPolicy<T, N, Align, SubAllocPolicy>::FreeListContainerPolicy(void* ptr) : SubAllocPolicy<N * sizeof(T), Align>(ptr) {
      reset_elements();
    }


    template<size_t N, size_t Align>
    StackContainerPolicy<N, Align, SubAllocPolicy>::StackContainerPolicy(ptr_t ptr) {
      top = alloc_t::allocate(ptr);
    }


// Bucket generation

    template<size_t BlockSize>
    struct Block {
      uint8_t block [BlockSize];
    };

    template<typename T, size_t BlockSize>
    struct block_adapter {
      using block_t = Block<BlockSize>;
      static Block<BlockSize> rebind(const T& chunk) {
        block_t block;
        memcpy(block.block, chunk, sizeof(T));
        return block;
      }
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
    using bucket_descriptors_t = typename BucketDescriptorSet<ID>::type;

    template<size_t ID>
    static constexpr size_t bucket_count = list_size<bucket_descriptors_t<ID>>::value;

    template<size_t ID, size_t Idx>
    struct get_size : Value<size_t, 
      meta::at_t<bucket_descriptors_t<ID>, Idx>::size>{};


    template<size_t ID, size_t Idx>
    struct get_count : Value<size_t, 
      meta::at_t<bucket_descriptors_t<ID>, Idx>::count>{};

    template<size_t ID, size_t Idx>
    struct bucket_size {
      static constexpr size_t value = get_size<ID, Idx>::value * get_count<ID, Idx>::value;
    };

    template<size_t ID, size_t Idx>
    struct get_set_size : get_set_size<ID, Idx-1>
    { 
      static constexpr size_t add = get_set_size<ID, Idx - 1>::add + bucket_size<ID, Idx>::value; 
    };

    template<size_t ID>
    struct get_set_size<ID, 0> 
    {
        static constexpr size_t add = bucket_size<ID, 0>::value;
    };

    template<size_t ID>
    struct set_size 
    {
        static constexpr size_t value = get_set_size<ID, bucket_count<ID> - 1>::add; 
    };

    template<size_t ID, size_t Idx>
    struct get_alignment : Value<size_t, 
      meta::at_t<bucket_descriptors_t<ID>, Idx>::align>{}; 


    template<size_t ID, size_t Idx>
    struct get_set_alignment_size : get_set_alignment_size<ID, Idx-1>
    { 
      static constexpr size_t add = get_set_alignment_size<ID, Idx - 1>::add + get_alignment<ID, Idx>::value; 
    };

    template<size_t ID>
    struct get_set_alignment_size<ID, 0> 
    {
        static constexpr size_t add = get_alignment<ID, 0>::value;
    };

    template<size_t ID>
    struct set_alignment_size 
    {
        static constexpr size_t value = get_set_alignment_size<ID, bucket_count<ID> - 1>::add; 
    };

    template<class Primary, class Fallback, bool Final = false>
    class FallbackBucketInserter 
      : private Primary
      , private Fallback 
    {
    public:
      using Primary::data;
      FallbackBucketInserter(void* init_list) 
        : Primary(init_list)
          , Fallback(
              (uint8_t*)init_list 
              + (Primary::get_block_size() * Primary::get_block_count()) 
              + Primary::get_block_size()
              ) {}

      const size_t get_block_size_at(void* ptr) {
        if(Primary::belongs(ptr)){
          return Primary::get_block_size();
        }
        return Fallback::get_block_size_at(ptr);
      }

      template<typename T>
      void* get_block() {
        if constexpr (sizeof(T) > Primary::get_block_size()) {
          return Fallback::template get_block<T>();
        }
        return Primary::add_object();
      }

      void free_block(void* ptr) {
        if(Primary::belongs(ptr)) {
          Primary::remove_object(ptr);
        } else { 
          Fallback::free_block(ptr);
        }
      }


      template<typename T>
      void return_block(T* ptr) {
        if constexpr (sizeof(T) > Primary::get_block_size())
          return Fallback::template return_block<T>(ptr);
        return Primary::remove_object(ptr);
      }

      template<size_t NewSize>
      static constexpr size_t get_block_size() {
        auto size = get_block_size();
        if(NewSize <= size) {
          return size;
        }
        return Fallback::template get_block_size<NewSize>();
      }

      static constexpr size_t get_block_size() {
        return Primary::get_block_size();
      }

      static constexpr size_t get_block_count() {
        return Primary::get_block_count();
      }

    };

    template<class Primary, class Fallback>
    class FallbackBucketInserter<Primary, Fallback, true> 
    : private Primary
    , private Fallback 
    {
    public:
      using Primary::data;
      FallbackBucketInserter(void* init_list) 
        : Primary(init_list)
          , Fallback(
              (uint8_t*)init_list 
              + (Primary::get_block_size() * Primary::get_block_count())
              + Primary::get_block_size()
              ) {}

      const size_t get_block_size_at(void* ptr) {
        if(!Primary::belongs(ptr)){
          if(!Fallback::belongs(ptr))
            return 0;
          return Fallback::get_block_size();
        } 
        return Primary::get_block_size();
      }

      template<typename T>
      void* get_block() {
        if constexpr (sizeof(T) > Primary::get_block_size()) {
          if constexpr (sizeof(T) > Fallback::get_block_size()) { 
            return nullptr;
          }
          return Fallback::add_object();
        }
        return Primary::add_object();
      }

      void free_block(void* ptr) {
        if(Primary::belongs(ptr)) {
          Primary::remove_object(ptr);
        } else if(Fallback::belongs(ptr)) {
          Fallback::remove_object(ptr);
        } else {
          L_ASSERT(false);
        }
      }


      template<typename T>
      void return_block(T* ptr) {
        if constexpr (sizeof(T) > Primary::get_block_size())
          return Fallback::remove_object(ptr);
        return Primary::remove_object(ptr);
      }

      template<size_t NewSize>
      static constexpr size_t get_block_size() {
        auto size = get_block_size();
        if(NewSize <= size) {
          return size;
        }
        return MAX_u64;
      }

      static constexpr size_t get_block_size() {
        return Primary::get_block_size();
      }

      static constexpr size_t get_block_count() {
        return Primary::get_block_count();
      }
    };

    template<size_t ID, size_t Count, size_t Idx = 0>
    struct fallback_inserter_generator;

    template<size_t ID, size_t Count, size_t Idx>
    struct fallback_inserter_generator {

      static constexpr bool Final = Idx + 1 == Count;

      using type = FallbackBucketInserter
        < Bucket
          < get_size<ID, Idx>::value
          , get_count<ID, Idx>::value
          , get_alignment<ID, Idx>::value
          >
        , typename fallback_inserter_generator
          < ID, Count, Idx + 1>::type
        , Final
        >;
    };

    template<size_t ID, size_t Count>
    struct fallback_inserter_generator<ID, Count, Count> {
     using type = Bucket<get_size<ID, Count>::value, get_count<ID, Count>::value, get_alignment<ID, Count>::value>;
    };

    template<size_t ID>
    struct fallback_inserter_generator<ID, 1>;

    template<size_t ID>
    using fallback_bucket_inserter_t = typename fallback_inserter_generator<ID, list_size<bucket_descriptors_t<ID>>::value>::type;

    template<size_t BlockSize, size_t BlockCount, size_t Alignment = BlockSize>
    using Bucket = FreeListContainerPolicy<Block<BlockSize>, BlockCount, Alignment, StackAllocPolicy>; 

    template<size_t ID>
    class MemoryPool : MAllocPolicy<set_size<ID>::value + set_alignment_size<ID>::value> {
    public:
      using alloc_policy_t = MAllocPolicy<set_size<ID>::value + set_alignment_size<ID>::value>;

      MemoryPool() : _inserter(alloc_policy_t::allocate()) {}

      ~MemoryPool() {}

      const size_t get_block_size_at(void* ptr) {
        return _inserter.get_block_size_at(ptr);
      }

      template<size_t NewSize>
      static constexpr size_t get_block_size() {
        return inserter_t::template get_block_size<NewSize>();
      }

      template<typename T>
      T* allocate() {
        return static_cast<T*>(_inserter.template get_block<T>());
      }

      void free_block_at(void* ptr) {
        _inserter.free_block(ptr);
      }

      template<typename T>
      void deallocate(T* ptr) {
        _inserter.template return_block<T>(ptr);
      }

    private:
      using inserter_t = 
        typename fallback_inserter_generator
        <
        ID, bucket_count<ID> - 1 
        >::type;

      inserter_t _inserter;

    };
    template<size_t ID>
    auto& get_pool() {
      static MemoryPool<ID> _pool{};
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
      using Bucket_1M = BucketDescriptor<1 << 20, Count, 1 << 8>; // 1 mega-byte aligned bucket
                                                                  
      template<size_t Count>
      using Bucket_16M = BucketDescriptor<1 << 24, Count, 1 << 8>; // 1 mega-byte aligned bucket




    }		// -----  end of namespace defaults  ----- 

  }		// -----  end of namespace mem  ----- 

  class Allocator {
  public:
    template<size_t ID, typename T>
    static void* allocate() {
      return mem::get_pool<ID>().template allocate<T>();
    }
    
    template<size_t ID, typename T>
    static void deallocate(T* ptr) {
      return mem::get_pool<ID>().template deallocate<T>(ptr);
    }

    template<size_t ID, typename T, typename U>
    static void* rebind(U* src) {
      auto new_ptr = mem::get_pool<ID>().template allocate<T>();
      auto size = CLAMP_TOP(sizeof(T), sizeof(U));
      MEM_COPY(new_ptr, src, size);
      mem::get_pool<ID>().template deallocate<U>(src);
      return new_ptr;
    }

    template<size_t ID, typename T>
    static constexpr size_t get_block_size() {
      return mem::get_pool<ID>().template get_block_size<sizeof(T)>();
    }
  };

  template<size_t ID>
  class RuntimeAllocator {
  public:
    static void* allocate(size_t size, size_t alignment = 0) {
      size = MAX(size, alignment);
      void* ptr = check_index<0>::alloc(size);
      L_ASSERT(ptr != nullptr);
      return ptr;
    }

    static void free(void* ptr) {
      if(ptr == nullptr)
        return;
      return mem::get_pool<ID>().free_block_at(ptr);
    }

    static void* reallocate(void* ptr, size_t size, size_t alignment) {
      size = MAX(size, alignment);
      void* new_ptr = check_index<0>::alloc(size);
      L_ASSERT(ptr != nullptr);
      auto old_size = get_block_size_at(ptr);
      old_size = MIN(size, old_size);
      MEM_COPY(new_ptr, ptr, old_size);
      free(ptr);
      return new_ptr;
    } 

    static size_t get_block_size_at(void* ptr) {
      return mem::get_pool<ID>().get_block_size_at(ptr);
    }

  private:
    template<size_t Index>
    struct check_index {
      static constexpr size_t Size = meta::at_t<mem::bucket_descriptors_t<ID>, Index>::size;
      using block_t = mem::Block<Size>;
      static void* alloc(size_t new_size) {
        if(new_size <= Size) {
          return mem::get_pool<ID>().template allocate<block_t>();
        } else {
          return check_index<Index + 1>::alloc(new_size);
        }
      }
    };

    template<>
    struct check_index<mem::bucket_count<ID>> {
      static void* alloc(size_t new_size) {
        return nullptr;
      }
    };
  };

  template<size_t ID, size_t InitialCap = MB(1), size_t Alignment = 8, template<size_t, size_t> class AllocPolicy = mem::SubAllocPolicy>
  class Arena; 

  template<size_t ID, size_t InitialCap, size_t Alignment>
  class Arena<ID, InitialCap, Alignment, mem::MAllocPolicy> : private mem::MAllocPolicy<InitialCap, Alignment> {
  public:
    using alloc_t = mem::MAllocPolicy<InitialCap, Alignment>;
    Arena() : alloc_t(), top(0) {}
    ~Arena() {
      alloc_t::allocate();
    }

    void* push(size_t size) {
      u8* result = (u8*)alloc_t::data();
      const size_t new_top = top + size;
      if(new_top > cap) {
        cap <<= 1;
        result = (u8*)alloc_t::move(realloc(alloc_t::data(), cap + Alignment));
      }
      result = (u8*)result + top;
      top = new_top;
      return result;
    }

    void pop_to(size_t pos) {
      if(pos >= top) {
        return;
      }
      top = pos;
      if(pos < (cap>>1)) {
        cap >>= 1;
        alloc_t::move(realloc(alloc_t::data(), cap + Alignment));
      }
    }

    void pop_amount(size_t amount) {
      size_t new_top = top - amount;
      pop_to(new_top);
    }

    const void* get_buffer() {
      return alloc_t::data();
    }

    template<size_t NewID, size_t NewCap, size_t NewAlignment>
    Arena& operator=(const Arena<NewID, NewCap, NewAlignment>& arena) {
      if(this == &arena) {
        return *this;
      }
      pop_to(0);
      auto ptr = push(arena.top);
      MEM_COPY(ptr, arena.get_buffer(), arena.top);
      return *this;
    }

  private:
    size_t cap = InitialCap;
    size_t top = 0;
  };


  template<size_t ID, size_t InitialCap, size_t Alignment>
  class Arena<ID, InitialCap, Alignment, mem::StackAllocPolicy> : private mem::StackAllocPolicy<InitialCap, Alignment> {
  public:
    using alloc_t = mem::StackAllocPolicy<InitialCap, Alignment>;
    Arena() : alloc_t(), top(0) {}
    ~Arena() {}

    void* push(size_t size) {
      u8* result = (u8*)alloc_t::data();
      const size_t new_top = top + size;
      L_ASSERT(new_top <= cap);
      result = (u8*)result + top;
      top = new_top;
      return result;
    }

    void pop_to(size_t pos) {
      if(pos >= top) {
        return;
      }
      top = pos;
    }

    void pop_amount(size_t amount) {
      size_t new_top = top - amount;
      pop_to(new_top);
    }

    const void* get_buffer() {
      return alloc_t::data();
    }

    template<size_t NewID, size_t NewCap, size_t NewAlignment>
    Arena& operator=(const Arena<NewID, NewCap, NewAlignment>& arena) {
      if(this == &arena) {
        return *this;
      }
      top = 0;
      auto ptr = push(arena.top);
      MEM_COPY(ptr, arena.get_buffer(), arena.top);

      return *this;
    }

  private:
    static constexpr size_t cap = InitialCap;
    size_t top = 0;
  };


  template<size_t ID, size_t InitialCap, size_t Alignment>
  class Arena<ID, InitialCap, Alignment, mem::SubAllocPolicy> : private mem::SubAllocPolicy<0, Alignment> {
  public:
    using alloc_t = mem::SubAllocPolicy<0, Alignment>;
    Arena() : alloc_t(Allocator::allocate<ID, mem::Block<InitialCap>>()), cap(Allocator::get_block_size<ID, mem::Block<InitialCap>>()), top(0) {}
    ~Arena() {
      RuntimeAllocator<ID>::free(alloc_t::data());
    }

    void* push(size_t size) {
      u8* result = (u8*)alloc_t::data();
      const size_t new_top = top + size;
      if(new_top > cap) {
        cap = check_size<0>::size(new_top);
        result = (u8*)alloc_t::move(RuntimeAllocator<ID>::reallocate(result, new_top, 8));
      }
      result = (u8*)result + top;
      top = new_top;
      return result;
    }

    void pop_to(size_t pos) {
      if(pos >= top) {
        return;
      }
      top = pos;
      if(check_size<0>::size(pos) < cap) {
        auto new_block = RuntimeAllocator<ID>::reallocate(alloc_t::data(), pos, Alignment);
        cap = RuntimeAllocator<ID>::get_block_size_at(new_block);
        alloc_t::move(new_block);
      }
    }

    void pop_amount(size_t amount) {
      size_t new_top = top - amount;
      pop_to(new_top);
    }

    const void* get_buffer() {
      return alloc_t::data();
    }

    template<size_t NewID, size_t NewCap, size_t NewAlignment>
    Arena& operator=(const Arena<NewID, NewCap, NewAlignment>& arena) {
      if(this == &arena) {
        return *this;
      }
      pop_to(0);
      auto ptr = push(arena.top);
      MEM_COPY(ptr, arena.get_buffer(), arena.top);

      return *this;
    }

  private:
    template<size_t Index>
    struct check_size {
      static constexpr size_t Size = meta::at_t<mem::bucket_descriptors_t<ID>, Index>::size;

      static constexpr size_t size(size_t new_size) {
        if(new_size <= Size) {
          return Size;
        }
        return check_size<Index + 1>::size(new_size);
      }
    };

    template<>
    struct check_size<mem::bucket_count<ID>> {
      static constexpr size_t size(size_t new_size) {
        return MAX_u64;
      }
    };

    size_t cap = 0;
    size_t top = 0;
  };

  namespace mem {
    using default_bucket_descriptor_set_t
          = List
            < defaults::Bucket_8<16384>
            , defaults::Bucket_64<16384>
            , defaults::Bucket_256<16384>
            , defaults::Bucket_1024<4096>
            , defaults::Bucket_16384<1024>
            , defaults::Bucket_1M<64>
            , defaults::Bucket_16M<16>>;

#if defined LOFI_DEFAULT_BUCKETS_COUNT
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

#endif
#else
      template<>
      struct BucketDescriptorSet<0> {
        using type = default_bucket_descriptor_set_t;
      };

#endif
  }		// -----  end of namespace mem  ----- 
}   // -----  end of namespace lofi  -----
