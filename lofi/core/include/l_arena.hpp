// =====================================================================================
//
//       Filename:  l_arena.hpp
//
//    Description:  
//
//        Version:  1.0
//        Created:  2025-02-14 8:34:21 PM
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
 
  template<size_t InitialCap = MB(1), size_t Alignment = 8, template<size_t, size_t> class AllocPolicy = mem::SubAllocPolicy>
  class Arena; 

  template<size_t Cap, size_t Alignment>
  class Arena<Cap, Alignment, mem::MAllocPolicy> : private mem::MAllocPolicy<Cap, Alignment> {
  public:
    using alloc_t = mem::MAllocPolicy<Cap, Alignment>;
    Arena() : alloc_t(), top(0) {
      alloc_t::allocate();
    }
    ~Arena() {
      alloc_t::deallocate();
    }

    void* push(size_t size) {
      u8* result = (u8*)alloc_t::data();
      const size_t new_top = top + size;
      if(new_top > Cap) {
        return nullptr;
      }
      result += top;
      top = new_top;
      return (void*)result;
    }

    void pop_to(const void* const pos) {
      const void* const base_ptr = alloc_t::data();
      if(pos >= (top + (u8*)base_ptr) || pos < base_ptr) {
        return;
      }
      top = PTR2INT(pos) - PTR2INT(base_ptr);
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

    const void* get_buffer() const {
      return alloc_t::data();
    }

    void clear() {
      top = 0;
    }
     
    const u32 get_size() {
      return top;
    }

    const u32 get_size() const {
      return top;
    }

  private:
    size_t top = 0;
  };


  template<size_t Cap, size_t Alignment>
  class Arena<Cap, Alignment, mem::StackAllocPolicy> : private mem::StackAllocPolicy<Cap, Alignment> {
  public:
    using alloc_t = mem::StackAllocPolicy<Cap, Alignment>;
    Arena() : alloc_t(), top(0) {}
    ~Arena() {}

    void* push(size_t size) {
      u8* result = (u8*)alloc_t::data();
      const size_t new_top = top + size;
      L_ASSERT(new_top <= Cap);
      result += top;
      top = new_top;
      return (void*)result;
    }

    void pop_to(const void* const pos) {
      const void* const base_ptr = alloc_t::data();
      if(pos >= (top + (u8*)base_ptr) || pos < base_ptr) {
        return;
      }
      top = PTR2INT(pos) - PTR2INT(base_ptr);
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

    const void* get_buffer() const {
      return alloc_t::data();
    }

    void clear() {
      top = 0;
    }
   
    const u32 get_size() {
      return top;
    }

    const u32 get_size() const {
      return top;
    }

  private:
    size_t top = 0;
  };


  template<size_t Cap, size_t Alignment>
  class Arena<Cap, Alignment, mem::SubAllocPolicy> : private mem::SubAllocPolicy<Cap, Alignment> {
  public:
    using alloc_t = mem::SubAllocPolicy<Cap, Alignment>;
    using index_t = typename alloc_t::index_t;
    Arena() {}
    Arena(void* ptr) : alloc_t(ptr), top(0) {}

    void move_ptr(void* ptr) {
      alloc_t::move(ptr);
    }

    ~Arena() {}

    void* push(size_t size) {
      u8* result = (u8*)alloc_t::data();
      const size_t new_top = top + size;
      if(new_top > Cap) {
        return nullptr;
      }
      result += top;
      top = new_top;
      return (void*)result;
    }

    void pop_to(const void* const pos) {
      const void* const base_ptr = alloc_t::data();
      if(pos >= (top + (u8*)base_ptr) || pos < base_ptr) {
        return;
      }
      top = PTR2INT(pos) - PTR2INT(base_ptr);
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

    const void* get_buffer() const {
      return alloc_t::data();
    }

    const u32 get_size() {
      return top;
    }

    const u32 get_size() const {
      return top;
    }

    void clear() {
      top = 0;
    }
  
  private:
    size_t top = 0;
  };


  template<size_t InitialCap = MB(1), size_t Alignment = 8, template<size_t, size_t> class AllocPolicy = mem::SubAllocPolicy>
  class LockFreeArena; 

  template<size_t InitialCap, size_t Alignment>
  class LockFreeArena<InitialCap, Alignment, mem::StackAllocPolicy> : mem::StackAllocPolicy<InitialCap, Alignment> {
  public:
    using alloc_t = mem::StackAllocPolicy<InitialCap, Alignment>;
    using index_t = typename alloc_t::index_t;

    void* push(size_t size) {
      const u64 current_top = top.add(size); 
      const u64 new_top = current_top + size;
      if(new_top > InitialCap) {
        return nullptr;
      }
      return (void*)((u8*)alloc_t::data() + current_top);
    }

//    unsafe for general case multithreaded programs
//
//    void pop_to(size_t pos) {
//      if(pos >= get_size()) {
//        return;
//      }
//      top = pos;
//    }
//
//    void pop_amount(size_t amount) {
//      size_t new_top = top - amount;
//      pop_to(new_top);
//    }

    const index_t get_size() const {
      return top.get_count();
    }


    const index_t get_size() {
      return top.get_count();
    }

    const void* get_buffer() {
      return alloc_t::data();
    }

    const void* get_buffer() const {
      return alloc_t::data();
    }

    void clear() {
      top = 0;
    }
  
  private:
    atomic_counter<InitialCap> top{0};
  };


  template<size_t InitialCap, size_t Alignment>
  class LockFreeArena<InitialCap, Alignment, mem::SubAllocPolicy> : mem::SubAllocPolicy<InitialCap, Alignment> {
  public:
    using alloc_t = mem::SubAllocPolicy<InitialCap, Alignment>;
    using index_t = typename alloc_t::index_t;

    LockFreeArena() : alloc_t{nullptr} {}

    LockFreeArena(void* ptr) : alloc_t{ptr} {}

    void move_ptr(void* ptr) {
      alloc_t::move(ptr);
    }

    void* push(size_t size) {
      const u64 current_top = top.add(size); 
      const u64 new_top = current_top + size;
      if(new_top > InitialCap) {
        return nullptr;
      }
      return (void*)((u8*)alloc_t::data() + current_top);
    }

//    unsafe for general case multithreaded programs
//
//    void pop_to(size_t pos) {
//      if(pos >= get_size()) {
//        return;
//      }
//      top = pos;
//    }
//
//    void pop_amount(size_t amount) {
//      size_t new_top = top - amount;
//      pop_to(new_top);
//    }

    const index_t get_size() const {
      return top.get_count();
    }

    const index_t get_size() {
      return top.get_count();
    }

    const void* get_buffer() {
      return alloc_t::data();
    }

    const void* get_buffer() const {
      return alloc_t::data();
    }

    void clear() {
      top = 0;
    }
  
  private:
    atomic_counter<InitialCap> top{0};
  };


  template<size_t InitialCap, size_t Alignment>
  class LockFreeArena<InitialCap, Alignment, mem::MAllocPolicy> : mem::MAllocPolicy<InitialCap, Alignment> {
  public:
    using alloc_t = mem::MAllocPolicy<InitialCap, Alignment>;
    using index_t = typename alloc_t::index_t;

    void* push(size_t size) {
      const u64 current_top = top.add(size); 
      const u64 new_top = current_top + size;
      if(new_top > InitialCap) {
        return nullptr;
      }
      return (void*)((u8*)alloc_t::data() + current_top);
    }

//    unsafe for general case multithreaded programs
//
//    void pop_to(size_t pos) {
//      if(pos >= get_size()) {
//        return;
//      }
//      top = pos;
//    }
//
//    void pop_amount(size_t amount) {
//      size_t new_top = top - amount;
//      pop_to(new_top);
//    }

    const index_t get_size() {
      return top.get_count();
    }

    const index_t get_size() const {
      return top.get_count();
    }

    const void* get_buffer() {
      return alloc_t::data();
    }

    const void* get_buffer() const {
      return alloc_t::data();
    }

    void clear() {
      top = 0;
    }
  
  private:
    atomic_counter<InitialCap> top{0};
  }; 

}		// -----  end of namespace lofi  ----- 
