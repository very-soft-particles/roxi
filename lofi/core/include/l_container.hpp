// =====================================================================================
//
//       Filename:  l_container.hpp
//
//    Description:  
//
//        Version:  1.0
//        Created:  2025-02-14 8:26:23 PM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Robyn Gray (rg), ...
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#pragma once
#include "l_memory.hpp"

namespace lofi {
  namespace mem {
    
      template<class T, size_t Size, size_t Alignment, template<size_t, size_t> class AllocPolicy>
      class PackedRingBufferContainerPolicy : AllocPolicy<Size * sizeof(T), Alignment> {
        public:
          using alloc_t = AllocPolicy<Size * sizeof(T), Alignment>;
          using alloc_t::data;
          using index_t = typename alloc_t::index_t;
          using value_t = T;
          using ptr_t = T*;
          using ref_t = T&;

          static constexpr index_t mask_value = Size - 1;
          static constexpr index_t null_value = index_type_max<index_t>::value;

          PackedRingBufferContainerPolicy() {
            alloc_t::allocate();
          }

          ~PackedRingBufferContainerPolicy() {}

          const T& operator[](const index_t index) const {
            return *get_ptr_cast(index);
          }

          T& operator[](const index_t index) {
            return *get_ptr_cast(index);
          }

          //b8 push(T src) {
          //  if(tail == null_value) {
          //    head = tail = 0;
          //  } else if (tail >= Size) {
          //    tail %= Size;
          //    if(tail >= head) {
          //      return false;
          //    }
          //  }
          //  *get_ptr_cast(tail++) = src;
          //  return true;
          //}

          b8 push(T* src) {
            if(tail == null_value) {
              head = tail = 0;
            } else if (tail >= Size) {
              tail &= mask_value;
              if(tail >= head) {
                return false;
              }
            }
            MEM_COPY(get_ptr_cast(tail++), src, sizeof(T));
            return true;
          }

          b8 push(const T& src) {
            if(tail == null_value) {
              head = tail = 0;
            } else if (tail >= Size) {
              tail &= mask_value;
              if(tail >= head) {
                return false;
              }
            }
            *get_ptr_cast(tail++) = src;
            return true;
          }

          b8 push(T&& src) {
            if(tail == null_value) {
              head = tail = 0;
            } else if (tail >= Size) {
              tail &= mask_value;
              if(tail >= head) {
                return false;
              }
            }
            *get_ptr_cast(tail++) = src;
            return true;
          }

          b8 pop(ptr_t dst) {
            if(head == null_value) {
              return false;
            }
            if(head >= Size) {
              head &= mask_value;
            }
            if(head >= tail) {
              return false;
            }
            MEM_COPY(dst, get_ptr_cast(head++), sizeof(T));
            return true;
          }

          b8 remove_object(const index_t index) {
            if(tail <= head) {
              if(index >= head || index < tail) {
                ptr_t dst = get_ptr_cast(index);
                ptr_t src = get_ptr_cast(tail--);
                MEM_COPY(dst, src, sizeof(T));
                return true;
              }
              return false;
            }
            if(index >= head && index < tail) {
              ptr_t dst = get_ptr_cast(index);
              ptr_t src = get_ptr_cast(tail--);
              MEM_COPY(dst, src, sizeof(T));
              return true;
            }
            return false;
          }

          index_t get_head() {
            return head;
          }

          index_t get_tail() {
            return tail;
          }
        private:

          ptr_t get_ptr_cast(const index_t index) {
            return (ptr_t)data() + index;
          }

          index_t head = null_value;
          index_t tail = null_value;
      };

    template<class T, size_t Size, size_t Alignment>
      class PackedRingBufferContainerPolicy<T, Size, Alignment, SubAllocPolicy> : SubAllocPolicy<MAX_u64, Alignment> {
        public:
          using alloc_t = SubAllocPolicy<MAX_u64, Alignment>;
          using alloc_t::data;
          using index_t = typename alloc_t::index_t;
          using value_t = T;
          using ptr_t = T*;
          using ref_t = T&;

          static constexpr index_t null_value = index_type_max<index_t>::value;

          PackedRingBufferContainerPolicy() {}

          PackedRingBufferContainerPolicy(const void* ptr, const u64 capacity) {
            _capacity = capacity;
            alloc_t::allocate(ptr);
          }

          ~PackedRingBufferContainerPolicy() {}

          const T& operator[](const index_t index) const {
            return *get_ptr_cast(index);
          }

          T& operator[](const index_t index) {
            return *get_ptr_cast(index);
          }

          //      b8 push(T src) {
          //        _size++;
          //        if(tail == null_value) {
          //          head = tail = 0;
          //        } else if (tail >= _capacity) {
          //          tail %= _capacity;
          //          if(tail >= head) {
          //            return false;
          //          }
          //        }
          //        *get_ptr_cast(tail++) = src;
          //        return true;
          //      }

          b8 push(T* src) {
            _size++;
            if(tail == null_value) {
              head = tail = 0;
            } else if (tail >= _capacity) {
              tail %= _capacity;
              if(tail >= head) {
                return false;
              }
            }
            MEM_COPY(get_ptr_cast(tail++), src, sizeof(T));
            return true;
          }

          b8 push(const T& src) {
            _size++;
            if(tail == null_value) {
              head = tail = 0;
            } else if (tail >= _capacity) {
              tail %= _capacity;
              if(tail >= head) {
                return false;
              }
            }
            *get_ptr_cast(tail++) = src;
            return true;
          }

          b8 push(T&& src) {
            _size++;
            if(tail == null_value) {
              head = tail = 0;
            } else if (tail >= _capacity) {
              tail %= _capacity;
              if(tail >= head) {
                return false;
              }
            }
            *get_ptr_cast(tail++) = src;
            return true;
          }

          b8 pop(ptr_t dst) {
            _size--;
            if(head == null_value) {
              return false;
            }
            if(head >= _capacity) {
              head %= _capacity;
            }
            if(head >= tail) {
              return false;
            }
            MEM_COPY(dst, get_ptr_cast(head++), sizeof(T));
            return true;
          }

          b8 remove_object(const index_t index) {
            if(tail <= head) {
              if(index >= head || index < tail) {
                ptr_t dst = get_ptr_cast(index);
                ptr_t src = get_ptr_cast(tail--);
                MEM_COPY(dst, src, sizeof(T));
                return true;
              }
              return false;
            }
            if(index >= head && index < tail) {
              ptr_t dst = get_ptr_cast(index);
              ptr_t src = get_ptr_cast(tail--);
              MEM_COPY(dst, src, sizeof(T));
              return true;
            }
            return false;
          }

          const index_t get_head() const {
            return head;
          }

          const index_t get_tail() const {
            return tail;
          }

          const index_t get_head() {
            return head;
          }

          const index_t get_tail() {
            return tail;
          }

          const index_t get_size() {
            return _size;
          }

          const index_t get_size() const {
            return _size;
          }
        private:
          u64 _size = 0;
          u64 _capacity = 0;

          ptr_t get_ptr_cast(const index_t index) {
            return (ptr_t)data() + index;
          }

          index_t head = null_value;
          index_t tail = null_value;
      };


    template<class T, size_t Size, size_t Alignment, template<size_t, size_t> class AllocPolicy>
      class PackedArrayContainerPolicy : AllocPolicy<Size * sizeof(T), Alignment> {
        public:
          using alloc_t = AllocPolicy<Size * sizeof(T), Alignment>;
          using alloc_t::data;
          using index_t = typename alloc_t::index_t;
          using value_t = T;
          using ptr_t = T*;
          using ref_t = T&;

          PackedArrayContainerPolicy() {
            alloc_t::allocate();
          }

          ~PackedArrayContainerPolicy() {}

          const T& operator[](const index_t index) const {
            return static_cast<T*>(alloc_t::data())[index];
          }

          T& operator[](const index_t index) {
            return static_cast<T*>(alloc_t::data())[index];
          }

          T* push(index_t count) {
            const index_t new_count = top + count;
            T* result = nullptr;
            if(new_count <= Size) {
              T* result = static_cast<T*>(&alloc_t::data())[top];
              top += count;
            }
            return result;
          }

          index_t add_object(T&& obj) {
            L_ASSERT(top != Size);
            static_cast<T*>(alloc_t::data())[top] = obj;
            return top++;
          }

          index_t add_object(T obj) {
            L_ASSERT(top != Size);
            static_cast<T*>(alloc_t::data())[top] = obj;
            return top++;
          }

          index_t add_object() {
            L_ASSERT(top != Size);
            return top++;
          }

          void remove_object(index_t index) {
            static_cast<T*>(alloc_t::data())[index] = static_cast<T*>(alloc_t::data())[--top];
          }

          const index_t get_size() const {
            return top;
          }

          index_t get_size() {
            return top;
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

          PackedArrayContainerPolicy() {};
          PackedArrayContainerPolicy(void* ptr) : alloc_t(ptr) {}

          ~PackedArrayContainerPolicy() {}

          const T& operator[](const index_t index) const {
            return static_cast<T*>(alloc_t::data())[index];
          }

          T& operator[](const index_t index) {
            return static_cast<T*>(alloc_t::data())[index];
          }

          void move_ptr(void* ptr) {
            clear();
            this->allocate(ptr);
          }

          T* push(index_t count) {
            T* result = static_cast<T*>(&alloc_t::data())[top];
            top += count;
            return result;
          }

          size_t add_object(T obj) {
            L_ASSERT(top != Size);
            static_cast<T*>(alloc_t::data())[top] = obj;
            return top++;
          }

          index_t add_object() {
            L_ASSERT(top != Size);
            return top++;
          }

          void remove_object(size_t index) {
            static_cast<T*>(alloc_t::data())[index] = static_cast<T*>(alloc_t::data())[--top];
          }

          void clear() {
            top = 0;
          }

          const size_t get_size() const {
            return top;
          }

          size_t get_size() {
            return top;
          }

          void* get_buffer() {
            return alloc_t::data();
          }

        private:
          index_t top = 0;
      };

    template<class TList, size_t Size, size_t Alignment, template<size_t, size_t> class AllocPolicy = SubAllocPolicy>
      class MultiArrayContainerPolicy;

    template<size_t Size, size_t Alignment, template<size_t, size_t> class AllocPolicy, typename... Ts>
      class MultiArrayContainerPolicy<List<Ts...>, Size, Alignment, AllocPolicy> : AllocPolicy<Size * meta::accumulate_sizes<Ts...>::value, Alignment> {
        public:
          using index_t = typename choose_index_type<Size>::type;
          using list_t = List<Ts...>;
          using multi_array_t = tuple<Ts[Size]...>;
          using ptr_t = multi_array_t*;
          static constexpr index_t AccumulatedListSize = meta::accumulate_sizes<Ts...>::value;
          using alloc_t = AllocPolicy<Size * AccumulatedListSize, Alignment>;

          MultiArrayContainerPolicy() {
            alloc_t::allocate();
          }

          template<u64 Index>
            using type_at = meta::at_t<list_t, Index>;

          template<u64 OuterIndex>
            type_at<OuterIndex>& at(index_t inner_index) {
              L_ASSERT(inner_index < top);
              return get_array_cast()->template get<OuterIndex>()[inner_index];
            }

          template<u64 OuterIndex>
            type_at<OuterIndex>* get() {
              return get_array_cast()->template get<OuterIndex>();
            }

          multi_array_t* get_multi_array() {
            return get_array_cast();
          }

          ptr_t push(index_t count) {
            index_t result = top;
            index_t new_top = top + count;
            L_ASSERT(new_top <= Size);
            top = new_top;
            return get_array_cast() + result;
          }

          index_t push_object(tuple<Ts...>& args) {
            meta::static_for(
                args, 
                [&]<u64 I>(IdxT<I> index, type_at<I>&& t) {
                get_array_cast()->template get<I>()[top] = t;
                }
                );
            return top++;
          }

          void pop(index_t count) {
            top -= count;
          }

          void clear() {
            top = 0;
          }

        private:
          multi_array_t* get_array_cast() {
            return (ptr_t)alloc_t::data();
          }

          index_t top = 0;
      };

    template<size_t Size, size_t Alignment, typename... Ts>
      class MultiArrayContainerPolicy<List<Ts...>, Size, Alignment, SubAllocPolicy> : SubAllocPolicy<Size * meta::accumulate_sizes<Ts...>::value, Alignment> {
        public:
          using index_t = typename choose_index_type<Size>::type;
          using list_t = List<Ts...>;
          using multi_array_t = tuple<Ts[Size]...>;
          using ptr_t = multi_array_t*;
          static constexpr index_t AccumulatedListSize = meta::accumulate_sizes<Ts...>::value;
          using alloc_t = SubAllocPolicy<Size * AccumulatedListSize, Alignment>;

          MultiArrayContainerPolicy(ptr_t ptr) {
            alloc_t::allocate(ptr);
          }

          void move_ptr(ptr_t ptr) {
            clear();
            this->allocate(ptr);
          }

          multi_array_t* get_multi_array() {
            return get_array_cast();
          }

          const index_t get_size() const {
            return top;
          }

          const index_t get_size() {
            return top;
          }

          template<u64 Index>
            using type_at = meta::at_t<list_t, Index>;

          template<u64 OuterIndex>
            type_at<OuterIndex>& at(index_t inner_index) {
              L_ASSERT(inner_index < top);
              return get_array_cast()->template get<OuterIndex>()[inner_index];
            }

          template<u64 OuterIndex>
            type_at<OuterIndex>* get() {
              return get_array_cast()->template get<OuterIndex>();
            }

          ptr_t push(index_t count) {
            index_t result = top;
            index_t new_top = top + count;
            L_ASSERT(new_top <= Size);
            top = new_top;
            return get_array_cast() + result;
          }

          index_t push_object(tuple<Ts...>& args) {
            meta::static_for(
                args, 
                [&]<u64 I>(IdxT<I> index, type_at<I>&& t) {
                get_array_cast()->template get<I>()[top] = t;
                }
                );
            return top++;
          }

          void pop(index_t count) {
            top -= count;
          }

          void clear() {
            top = 0;
          }

        private:
          multi_array_t* get_array_cast() {
            return (ptr_t)alloc_t::data();
          }

          index_t top = 0;
      };


    template<class TList, size_t Size, size_t Alignment, template<size_t, size_t> class AllocPolicy = SubAllocPolicy>
      class PackedMultiArrayContainerPolicy;

    template<size_t Size, size_t Alignment, template<size_t, size_t> class AllocPolicy, typename... Ts>
      class PackedMultiArrayContainerPolicy<List<Ts...>, Size, Alignment, AllocPolicy> : AllocPolicy<Size * meta::accumulate_sizes<Ts...>::value, Alignment> {
        public:
          using index_t            = typename choose_index_type<Size>::type;
          using list_t             = List<Ts...>;
          using multi_array_list_t = List<Ts[Size]...>;
          using multi_array_t      = tuple<Ts[Size]...>;
          using ptr_t              = multi_array_t*;
          static constexpr index_t 
            AccumulatedListSize    = meta::accumulate_sizes<Ts...>::value;

          using alloc_t            = AllocPolicy<Size * AccumulatedListSize, Alignment>;

        private:
          template<typename... Idxs>
            using offset_seq_t = List<get_offset<Idxs::value, multi_array_list_t>...>;
          using apply_offset_seq_t = typename meta::lift<offset_seq_t, typename IdxSequence<sizeof...(Ts)>::type>;
          static constexpr auto type_offset_array = index_array<typename apply_offset_seq_t::type>;

        public:
          PackedMultiArrayContainerPolicy() {
            alloc_t::allocate();
          }

          template<u64 Index>
            using type_at = meta::at_t<list_t, Index>;

          multi_array_t* get_multi_array() {
            return get_array_cast();
          }

          template<u64 OuterIndex>
            type_at<OuterIndex>& at(index_t inner_index) {
              L_ASSERT(inner_index < top);
              return get_array_cast()->template get<OuterIndex>()[inner_index];
            }

          template<u64 OuterIndex>
            type_at<OuterIndex>* get() {
              return get_array_cast()->template get<OuterIndex>();
            }

          const void* operator[](const index_t outer_index) const {
            u8* ptr = (u8*)alloc_t::data() + type_offset_array[outer_index];
            return (void*)ptr;
          }

          void* operator[](const index_t outer_index) {
            u8* ptr = (u8*)alloc_t::data() + type_offset_array[outer_index];
            return (void*)ptr;
          }

          index_t add_object() {
            L_ASSERT(top < Size);
            return top++;
          }

          index_t add_object(tuple<Ts...>&& args) {
            meta::static_for(
                FWD(args), 
                [&]<u64 I>(IdxT<I> index, type_at<I>&& t) {
                get_array_cast()->template get<I>()[top] = t;
                }
                );
            return top++;
          }

          void remove_object(index_t index) {
            auto& array = *get_array_cast();
            meta::static_for(
                array,
                [&]<u64 I>(IdxT<I>, type_at<I>* t) {
                t[index] = t[top - 1];
                }
                );
            top--;
          }

          void clear() {
            top = 0;
          }

          const index_t get_size() const {
            return top;
          }

          const index_t get_size() {
            return top;
          }

        private:
          multi_array_t* get_array_cast() {
            return (ptr_t)alloc_t::data();
          }

          multi_array_t& get_ref_cast() {
            return *((ptr_t)alloc_t::data());
          }



          index_t top = 0;
      };

    template<size_t Size, size_t Alignment, typename... Ts>
      class PackedMultiArrayContainerPolicy<List<Ts...>, Size, Alignment, SubAllocPolicy> : SubAllocPolicy<Size * meta::accumulate_sizes<Ts...>::value, Alignment> {
        public:
          using index_t = typename choose_index_type<Size>::type;
          using list_t = List<Ts...>;
          using multi_array_t = tuple<Ts[Size]...>;
          using multi_array_list_t = List<Ts[Size]...>;
          using ptr_t = multi_array_t*;
          static constexpr index_t AccumulatedListSize = meta::accumulate_sizes<Ts...>::value;
          using alloc_t = SubAllocPolicy<Size * AccumulatedListSize, Alignment>;

        private:
          template<typename... Idxs>
            using offset_seq_t = List<get_offset<Idxs::value, multi_array_list_t>...>;
          using apply_offset_seq_t = typename meta::lift<offset_seq_t, IdxSequence<sizeof...(Ts)>>;
          static constexpr auto type_offset_array = index_array<typename apply_offset_seq_t::type>;

          PackedMultiArrayContainerPolicy(ptr_t ptr) {
            alloc_t::allocate(ptr);
          }

          void move_ptr(ptr_t ptr) {
            clear();
            this->allocate(ptr);
          }

          multi_array_t* get_multi_array() {
            return get_array_cast();
          }

          template<u64 Index>
            using type_at = meta::at_t<list_t, Index>;

          template<u64 OuterIndex>
            type_at<OuterIndex>& at(index_t inner_index) {
              L_ASSERT(inner_index < top);
              return get_array_cast()->template get<OuterIndex>()[inner_index];
            }

          template<u64 OuterIndex>
            type_at<OuterIndex>* get() {
              return &get_array_cast()->template get<OuterIndex>();
            }

          const void* operator[](const index_t outer_index) const {
            u8* ptr = (u8*)alloc_t::data() + type_offset_array[outer_index];
            return (void*)ptr;
          }

          void* operator[](const index_t outer_index) {
            u8* ptr = (u8*)alloc_t::data() + type_offset_array[outer_index];
            return (void*)ptr;
          }

          index_t add_object() {
            L_ASSERT(top < Size);
            return top++;
          }

          index_t add_object(tuple<Ts...>&& args) {
            meta::static_for(
                FWD(args), 
                [&]<u64 I>(IdxT<I>, type_at<I>& t) {
                get_array_cast().template get<I>()[top] = t;
                }
                );
            return top++;
          }

          void remove_object(index_t index) {
            meta::static_for(
                get_array_cast(),
                [&]<u64 I>(IdxT<I>, type_at<I>& t) {
                t[index] = t[top - 1];
                }
                );
            top--;
          }

          void clear() {
            top = 0;
          }

          const index_t get_size() const {
            return top;
          }

          const index_t get_size() {
            return top;
          }

        private:
          ptr_t get_array_cast() {
            return (ptr_t)alloc_t::data();
          }

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
            alloc_t::allocate();
            reset_indices();
          }

          ~SparseArrayContainerPolicy() {}

          const T& operator[](const index_t index) const {
            return static_cast<T*>(alloc_t::data())[index];
          }

          T& operator[](const index_t index) {
            return static_cast<T*>(alloc_t::data())[index];
          }

          index_t add_object() {
            L_ASSERT(top != UINT32_MAX);
            auto here = *get_handle(top);
            auto temp = here;

            here = top;
            top = temp;

            return here;
          }


          index_t add_object(T obj) {
            L_ASSERT(top != UINT32_MAX);
            auto here = *get_handle(top);
            auto temp = here;

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

          void reset() {
            reset_indices();
          }

        private:
          void reset_indices() {
            top = 0;
            for(index_t i = 0; i < Size; i++) {
              const u64 next = i + 1;
              index_t* temp = get_handle(i);
              if(next == Size) {
                *temp = index_type_max<index_t>::value;
                break;
              }
              *temp = next;
            }
          }

          inline index_t* get_handle(const index_t index) {
            return ((index_t*)(static_cast<T*>(alloc_t::data()) + index));
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


          SparseArrayContainerPolicy() : alloc_t(nullptr) {};

          SparseArrayContainerPolicy(void* ptr) : alloc_t(ptr) {
            static_assert(sizeof(T) >= sizeof(index_t), "size of T too small in SparseArray");
            reset_indices();
          }

          ~SparseArrayContainerPolicy() {}

          const T& operator[](const index_t index) const {
            return static_cast<T*>(alloc_t::data())[index];
          }

          T& operator[](const index_t index) {
            return static_cast<T*>(alloc_t::data())[index];
          }

          void move_ptr(void* ptr) {
            this->move(ptr);
          }

          index_t add_object() {
            L_ASSERT(top != UINT32_MAX);
            auto here = *get_handle(top);
            auto temp = here;

            here = top;
            top = temp;

            return here;
          }

          index_t add_object(T obj) {

            L_ASSERT(top != UINT32_MAX);
            auto here = *get_handle(top);
            auto temp = here;

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

          void* get_buffer() {
            return alloc_t::data();
          }

          void reset() {
            reset_indices();
          }

        private:
          void reset_indices() {
            top = 0;
            for(size_t i = 0; i < Size; i++) {
              if(i + 1 == Size) {
                *get_handle(i) = UINT32_MAX;
                break;
              }
              *get_handle(i) = i + 1;
            }
          }

          inline index_t* get_handle(const index_t index) {
            return ((index_t*)(static_cast<T*>(alloc_t::data()) + index));
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
      class LinearMap : AllocPolicy<(sizeof(T) + sizeof(KeyType)) * Size, Alignment> {
        public:
          using alloc_t = AllocPolicy<(sizeof(T) + sizeof(KeyType)) * Size, Alignment>;
          static constexpr KeyType null_key = choose_null_key<KeyType>::value;


          LinearMap() {
            static_assert(((Size - 1) & (~Size)) == (Size - 1), "Size of map is not a power of two");
            reset_keys();
          }

          ~LinearMap() {
          }

          T& operator[](const KeyType index) {
            const KeyType true_key = find_key(index);
            L_ASSERT(true_key != null_key && "tried to index a null_key in LinearMap, use has(index) to ensure the value already exists");
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
      class LinearMap<T, Alignment, SubAllocPolicy, KeyType, Size> : SubAllocPolicy<(sizeof(T) + sizeof(KeyType)) * Size> {
        public:
          using alloc_t = SubAllocPolicy<(sizeof(T) + sizeof(KeyType)) * Size>;
          static constexpr KeyType null_key = choose_null_key<KeyType>::value;

          LinearMap() = delete;

          LinearMap(void* ptr) : alloc_t(ptr) {
            static_assert(((Size - 1) & (~Size)) == (Size - 1), "Size of map is not a power of two");
            reset_keys();
          }

          ~LinearMap() {
          }

          T& operator[](const KeyType index) {
            const KeyType true_key = find_key(index);
            L_ASSERT(true_key != null_key);
            return get_values()[true_key];
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


    template<class T, class KeyType = uint32_t>
      class DynamicMap : SubAllocPolicy<0> {
        private:
          u32 _size;
        public:
          using alloc_t = SubAllocPolicy<0>;
          static constexpr KeyType null_key = choose_null_key<KeyType>::value;

          DynamicMap(const u32 size) {
            L_ASSERT(((size - 1) & (~size)) == (size - 1) && "Size of map is not a power of two");
            _size = size;
          }

          ~DynamicMap() {
          }

          T& operator[](const KeyType index) {
            const KeyType true_key = find_key(index);
            L_ASSERT(true_key != null_key && "tried to index a null_key in Map, use has(index) to ensure the value already exists");
            return get_values()[true_key];
          }

          KeyType insert(const uint32_t key, const T t) {
            auto new_key = get_new_key(key);
            if(new_key == null_key)
              return null_key;
            get_keys()[new_key] = key;
            get_values()[new_key] = t;
            return new_key;
          }

          void remove(const KeyType key) {
            auto new_key = find_key(key);
            set_key_to_null(new_key);
          }

          bool has(const uint32_t key) {
            return find_key(key) != null_key;
          }

          void move_ptr(void* ptr) {
            this->move(ptr);
          }
          void reset() {
            reset_keys();
          }

        private:
          void reset_keys() {
            for(size_t i = 0; i < _size; i++) {
              set_key_to_null(i);
            }
          }

          KeyType find_key(const KeyType key) {
            uint32_t acc = key & (_size - 1);
            auto count = _size;

            while(count--) {
              if(get_keys()[acc] == key)
                return acc;
              ++acc;
              acc &= (_size - 1);
            }

            return null_key;
          }

          KeyType get_new_key(const KeyType key) {
            KeyType acc = key & (_size - 1);
            auto count = _size;

            while(count--) {
              if(key_is_null(acc))
                return acc;
              ++acc;
              acc &= (_size - 1);
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
            return (KeyType*)(alloc_t::data());
          }

          inline T* get_values() {
            return (T*)(get_keys() + _size);
          }

      };

    template<class T, size_t Alignment, template<size_t, size_t> class AllocPolicy, class KeyType = uint32_t, uint32_t Size = 64>
      class HashMap : AllocPolicy<(sizeof(T) + sizeof(KeyType)) * Size, Alignment> {
        public:
          using alloc_t = AllocPolicy<(sizeof(T) + sizeof(KeyType)) * Size, Alignment>;
          static constexpr KeyType null_key = choose_null_key<KeyType>::value;

          HashMap() {
            reset_keys();
          }

          ~HashMap() {
          }

          const T& operator[](const KeyType index) const {
            const KeyType true_key = find_key(index);
            L_ASSERT(true_key != null_key && "tried to index a null_key in HashMap, use has(index) to ensure the value already exists");
            return get_values(true_key);
          }

          T& operator[](const KeyType index) {
            const KeyType true_key = find_key(index);
            L_ASSERT(true_key != null_key && "tried to index a null_key in HashMap, use has(index) to ensure the value already exists");
            return get_values(true_key);
          }

          KeyType insert(const KeyType key, const T t) {
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

          const b8 has(const KeyType key) const {
            return find_key(key) != null_key;
          }

          const b8 has(const KeyType key) {
            return find_key(key) != null_key;
          }

          const void* get_buffer() {
            return alloc_t::data();
          }

        private:
          void reset_keys() {
            for(size_t i = 0; i < Size; i++) {
              set_key_to_null(i);
            }
          }

          u64 find_key(const KeyType key) {
            const u64 hashed_key = helpers::Hasher<KeyType>::hash(key);
            uint32_t acc = hashed_key % Size;
            auto count = Size;

            while(count--) {
              if(get_keys()[acc] == key)
                return acc;
              ++acc;
              acc %= Size;
            }

            return MAX_u64;
          }

          u64 get_new_key(const KeyType key) {
            const u64 hashed_key = helpers::Hasher<KeyType>::hash(key);
            KeyType acc = hashed_key % Size;
            auto count = Size;

            while(count--) {
              if(key_is_null(acc))
                return acc;
              ++acc;
              acc %= Size;
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

    template<class T, size_t Alignment, template<size_t, size_t> class AllocPolicy, uint32_t Size>
      class HashMap<T, Alignment, AllocPolicy, String, Size> : AllocPolicy<(sizeof(T) + sizeof(String)) * Size, Alignment> {
        public:
          using alloc_t = AllocPolicy<(sizeof(T) + sizeof(String)) * Size, Alignment>;

          static constexpr u64 null_key = index_type_max<u64>::value;

          HashMap() {
            reset_keys();
          }

          ~HashMap() {
          }

          const T& operator[](const String index) const {
            const auto idx = find_key(index);
            L_ASSERT(idx != null_key && "tried to index a null_key in HashMap, use has(index) to ensure the value already exists");
            return get_values(idx);
          }

          T& operator[](const String index) {
            const auto idx = find_key(index);
            L_ASSERT(idx != null_key && "tried to index a null_key in HashMap, use has(index) to ensure the value already exists");
            return get_values(idx);
          }

          String insert(const String key, const T t) {
            auto idx = get_new_key(key);
            if(idx == null_key)
              return {};
            get_keys()[idx] = key;
            get_values()[idx] = t;
          }

          void remove(const String key) {
            auto idx = find_key(key);
            set_key_to_null(idx);
          }

          const b8 has(const String key) const {
            return find_key(key) != null_key;
          }

          const b8 has(const String key) {
            return find_key(key) != null_key;
          }

          const void* get_buffer() {
            return alloc_t::data();
          }

        private:
          void reset_keys() {
            for(size_t i = 0; i < Size; i++) {
              set_key_to_null(i);
            }
          }

          u64 find_key(const String key) {
            const u64 hashed_key = helpers::Hasher<String>::hash(key);
            u64 acc = hashed_key % Size;
            auto count = Size;

            while(count--) {
              if(str_compare(get_keys()[acc], key))
                return acc;
              ++acc;
              acc %= Size;
            }

            return null_key;
          }

          u64 get_new_key(const String key) {
            const u64 hashed_key = helpers::Hasher<String>::hash(key);
            u64 acc = hashed_key % Size;
            auto count = Size;

            while(count--) {
              if(key_is_null(acc))
                return acc;
              ++acc;
              acc %= Size;
            }

            return null_key;
          }

          inline bool key_is_null(const u64 key) {
            return get_keys()[key] == null_key;
          }

          inline void set_key_to_null(const u64 key) {
            get_keys()[key] = null_key;
          }

          inline String* get_keys() {
            return static_cast<String*>(alloc_t::data());
          }

          inline T* get_values() {
            return static_cast<T*>(get_keys() + Size);
          }
      };


    template<class T, size_t Alignment, class KeyType, uint32_t Size>
      class HashMap<T, Alignment, SubAllocPolicy, KeyType, Size> : SubAllocPolicy<MAX_u64, Alignment> {
        public:
          using alloc_t = SubAllocPolicy<MAX_u64, Alignment>;
          static constexpr KeyType null_key = choose_null_key<KeyType>::value;

          HashMap() {}

          HashMap(const void* ptr, const u64 size) {
            _size = size;
            this->move(ptr);
            reset_keys();
          }

          void move_ptr_and_reset(const void* ptr, const u64 size) {
            _size = size;
            this->move(ptr);
            reset_keys();
          }

          ~HashMap() {
          }

          const T& operator[](const KeyType index) const {
            const KeyType true_key = find_key(index);
            L_ASSERT(true_key != null_key && "tried to index a null_key in HashMap, use has(index) to ensure the value already exists");
            return get_values()[true_key];
          }

          T& operator[](const KeyType index) {
            const KeyType true_key = find_key(index);
            L_ASSERT(true_key != null_key && "tried to index a null_key in HashMap, use has(index) to ensure the value already exists");
            return get_values()[true_key];
          }

          KeyType insert(const KeyType key, const T t) {
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

          const b8 has(const KeyType key) const {
            return find_key(key) != null_key;
          }

          const b8 has(const KeyType key) {
            return find_key(key) != null_key;
          }

          const void* get_buffer() {
            return alloc_t::data();
          }

        private:
          u64 _size = 0; 
          void reset_keys() {
            for(size_t i = 0; i < _size; i++) {
              set_key_to_null(i);
            }
          }

          const u64 find_key(const KeyType key) const {
            const u64 hashed_key = helpers::Hasher<KeyType>::hash(key);
            uint32_t acc = hashed_key % Size;
            auto count = Size;

            while(count--) {
              if(get_keys()[acc] == key)
                return acc;
              ++acc;
              acc %= Size;
            }

            return MAX_u64;
          }


          const u64 find_key(const KeyType key) {
            const u64 hashed_key = helpers::Hasher<KeyType>::hash(key);
            uint32_t acc = hashed_key % Size;
            auto count = Size;

            while(count--) {
              if(get_keys()[acc] == key)
                return acc;
              ++acc;
              acc %= Size;
            }

            return MAX_u64;
          }

          u64 get_new_key(const KeyType key) {
            const u64 hashed_key = helpers::Hasher<KeyType>::hash(key);
            KeyType acc = hashed_key % Size;
            auto count = Size;

            while(count--) {
              if(key_is_null(acc))
                return acc;
              ++acc;
              acc %= Size;
            }

            return MAX_u64;
          }

          inline bool key_is_null(const KeyType key) const {
            return get_keys()[key] == null_key;
          }

          inline void set_key_to_null(const KeyType key) const {
            get_keys()[key] = null_key;
          }

          inline KeyType* get_keys() const {
            return static_cast<KeyType*>(alloc_t::data());
          }

          inline T* get_values() const {
            return static_cast<T*>(get_keys() + Size);
          }

          inline bool key_is_null(const KeyType key) {
            return get_keys()[key] == null_key;
          }

          inline void set_key_to_null(const KeyType key) {
            get_keys()[key] = null_key;
          }

          inline KeyType* get_keys() {
            return (KeyType*)(alloc_t::data());
          }

          inline T* get_values() {
            return (T*)(get_keys() + Size);
          }
      };


    template<class T, size_t N, size_t Align, template<size_t, size_t> class AllocPolicy = SubAllocPolicy>
      class DoubleBufferContainerPolicy : private AllocPolicy<N * sizeof(T) * 2, Align> {
        public:
          using alloc_t = AllocPolicy<N * sizeof(T), Align>;
          using alloc_t::data;
          using alloc_t::belongs;
          using value_t = T;
          using ptr_t = T*;
          using ref_t = T&;
          using index_t = typename choose_index_type<N * 2>::type;

          DoubleBufferContainerPolicy() {
            current_write_buffer = 0;
            top[0] = bottom[0];
            top[1] = bottom[1];
          }

          ptr_t write() {
            if(top[current_write_buffer] == bottom[current_write_buffer] + HalfSize)
              return nullptr;
            return &get_ptr_cast()[top[current_write_buffer]++];
          }

          ptr_t read(index_t* count) {
            *count = top[!current_write_buffer] - bottom[!current_write_buffer];
            return &get_ptr_cast()[bottom[!current_write_buffer]];
          }

          index_t current_read_buffer_size() const {
            return top[!current_write_buffer];
          }

          void swap_buffers() {
            current_write_buffer ^= 1;
          }

          void clear_writes() {
            top[current_write_buffer] = bottom[current_write_buffer];
          }

          void clear_reads() {
            top[!current_write_buffer] = bottom[!current_write_buffer];
          }

          void clear_all() {
            top[0] = bottom[0];
            top[1] = bottom[1];
          }

        private:
          ptr_t get_ptr_cast() {
            return (ptr_t)alloc_t::data();
          }
          static constexpr index_t HalfSize = N/2;
          static constexpr index_t bottom[] {0,HalfSize};
          index_t top[2];
          index_t current_write_buffer;
      };

    template<class T, size_t N, size_t Align>
      class DoubleBufferContainerPolicy<T, N, Align, SubAllocPolicy> : private SubAllocPolicy<N * sizeof(T), Align> {
        public:
          using alloc_t = SubAllocPolicy<N * sizeof(T), Align>;
          using alloc_t::data;
          using alloc_t::belongs;
          using value_t = T;
          using ptr_t = T*;
          using ref_t = T&;
          using index_t = typename choose_index_type<N * 2>::type;

          DoubleBufferContainerPolicy(void* ptr) : alloc_t(ptr) {
            current_write_buffer = 0;
            top[0] = bottom[0];
            top[1] = bottom[1];
          }

          DoubleBufferContainerPolicy() : alloc_t(nullptr) {
            current_write_buffer = 0;
            top[0] = bottom[0];
            top[1] = bottom[1];
          }

          void move_ptr(void* ptr) {
            this->move(ptr);
          }

          ptr_t write() {
            if(top[current_write_buffer] == bottom[current_write_buffer] + HalfSize)
              return nullptr;
            return &get_ptr_cast()[top[current_write_buffer]++];
          }

          ptr_t read(index_t* count) {
            *count = top[!current_write_buffer] - bottom[!current_write_buffer];
            return &get_ptr_cast()[bottom[!current_write_buffer]];
          }

          index_t current_read_buffer_size() const {
            return top[!current_write_buffer];
          }

          void swap_buffers() {
            current_write_buffer ^= 1;
          }

          void clear_writes() {
            top[current_write_buffer] = bottom[current_write_buffer];
          }

          void clear_reads() {
            top[!current_write_buffer] = bottom[!current_write_buffer];
          }

          void clear_all() {
            top[0] = 0;
            top[1] = N;
          }
        private:
          ptr_t get_ptr_cast() {
            return (ptr_t)alloc_t::data();
          }
          static constexpr index_t HalfSize = N/2;
          static constexpr index_t bottom[] {0,HalfSize};
          index_t top[2];
          index_t current_write_buffer;
      };

    template<class T, size_t N, size_t Align, template<size_t, size_t> class AllocPolicy = SubAllocPolicy>
      class FreeListContainerPolicy : private AllocPolicy<N * sizeof(T), Align> {
        public:
          using alloc_t = AllocPolicy<N * sizeof(T), Align>;
          using alloc_t::data;
          using alloc_t::belongs;
          using value_t = T;
          using ptr_t = T*;
          using ref_t = T&;

          FreeListContainerPolicy() {
            reset_elements();
          }


          ~FreeListContainerPolicy(){};

          ptr_t add_object(T obj) {
            auto thisone = (T**)top;
            if(thisone == nullptr) {
              return nullptr;
            }
            auto temp = *thisone;
            top = temp;

            *thisone = obj;

            return (T*)thisone;
          }

          ptr_t add_object() {
            auto thisone = (T**)top;
            if(thisone == nullptr) {
              return nullptr;
            }
            auto temp = *thisone;
            top = temp;

            return (T*)thisone;
          }

          bool remove_object(void* rhs) {
            if(!alloc_t::belongs((void*)rhs))
              return false;
            ptr_t temp = top;
            top = (ptr_t)rhs;
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
              const size_t next = i + 1;
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
          using alloc_t::data;
          using alloc_t::belongs;
          using value_t = T;
          using ptr_t = T*;
          using ref_t = T&;
          FreeListContainerPolicy() : alloc_t(nullptr) {}
          FreeListContainerPolicy(void* ptr);

          void move_ptr(void* ptr) {
            top = (ptr_t)this->move(ptr);
            reset_elements();
          }

          ptr_t add_object() {
            auto thisone = (T**)top;
            if(thisone == nullptr) {
              return nullptr;
            }
            auto temp = *thisone;
            top = temp;
            return (T*)thisone;
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
          using const_ref_t = const T&;
          using index_t = typename choose_index_type<N>::type;
          using alloc_t = AllocPolicy<N * sizeof(T), Align>;

          ArrayContainerPolicy() : alloc_t() {
            alloc_t::allocate();
          }

          const_ref_t operator[](const index_t index) const {
            return get_const_array_cast()[index];
          }

          ref_t operator[](const index_t index) {
            return get_array_cast()[index];
          }

          const index_t get_size() const {
            return top;
          }

          const index_t get_size() {
            return top;
          }

          ptr_t get_buffer() {
            return get_array_cast();
          }

          ptr_t get_buffer() const {
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

          void clear() {
            top = 0;
          }

        private:
          const T* get_const_array_cast() const {
            return (const ptr_t)alloc_t::data();
          }

          T* get_array_cast() const {
            return (ptr_t)alloc_t::data();
          }

          T* get_array_cast() {
            return (ptr_t)alloc_t::data();
          }
          index_t top = 0;
      };

    template<class T, size_t Align>
      class ArrayContainerPolicy<T, 0, Align, StackAllocPolicy> {
        public:
          using value_t = T;
          using ptr_t = T*;
          using ref_t = T&;
          using const_ref_t = const T&;
          using index_t = typename choose_index_type<0>::type;
          using alloc_t = void;

          ArrayContainerPolicy() {}

          const index_t get_size() {
            return 0;
          }

          const index_t get_size() const {
            return 0;
          }

          ptr_t push(index_t count) {
            return nullptr;
          }

          static constexpr index_t get_cap() {
            return 0;
          }

          void pop(index_t count) {}

          void clear() {}

        private:
      };


    template<class T, size_t N, size_t Align>
      class ArrayContainerPolicy<T, N, Align, SubAllocPolicy> : SubAllocPolicy<N * sizeof(T)
      , Align> {
        public:
          using value_t = T;
          using ptr_t = T*;
          using ref_t = T&;
          using const_ref_t = const T&;
          using index_t = typename choose_index_type<N>::type;
          using alloc_t = SubAllocPolicy<N * sizeof(T), Align>;

          ArrayContainerPolicy() : alloc_t(nullptr) {}

          ArrayContainerPolicy(void* ptr) : alloc_t((ptr_t)ptr) {}

          void move_ptr(void* ptr) {
            this->move(ptr);
          }

          const_ref_t operator[](const index_t index) const {
            return get_const_array_cast()[index];
          }

          ref_t operator[](const index_t index) {
            return get_array_cast()[index];
          }

          const index_t get_size() {
            return top;
          }

          const index_t get_size() const {
            return top;
          }

          ptr_t get_buffer() {
            return get_array_cast();
          }

          ptr_t get_buffer() const {
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

          void clear() {
            top = 0;
          }

        private:
          const T* get_const_array_cast() const {
            return (const ptr_t)alloc_t::data();
          }

          T* get_array_cast() const {
            return (ptr_t)alloc_t::data();
          }

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



  }		// -----  end of namespace mem  ----- 
}		// -----  end of namespace lofi  ----- 
