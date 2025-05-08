// =====================================================================================
//
//       Filename:  rx_container.hpp
//
//    Description:  
//
//        Version:  1.0
//        Created:  2025-02-15 11:38:33 AM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Robyn Gray (rg), ...
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#pragma once
#include "rx_vocab.h"
#include "../../../lofi/core/include/l_container.hpp"

namespace roxi {
  template<typename T>
  using StackArray = lofi::mem::ArrayContainerPolicy<T, DefaultStackArraySize, DefaultAlignment, lofi::mem::StackAllocPolicy>;
 
  template<typename T>
  using HeapArray = lofi::mem::ArrayContainerPolicy<T, DefaultHeapArraySize, DefaultAlignment, lofi::mem::MAllocPolicy>;
 
  template<typename T>
  using Array = lofi::mem::ArrayContainerPolicy<T, DefaultArraySize, DefaultAlignment, lofi::mem::SubAllocPolicy>;
 
  template<typename T, u64 Size, u64 Align = DefaultAlignment>
  using SizedStackArray = lofi::mem::ArrayContainerPolicy<T, Size, Align, lofi::mem::StackAllocPolicy>;
 
  template<typename T, u64 Size, u64 Align = DefaultAlignment>
  using SizeHeapArray = lofi::mem::ArrayContainerPolicy<T, Size, Align, lofi::mem::MAllocPolicy>;
 
  template<typename T, u64 Size, u64 Align = DefaultAlignment>
  using SizedArray = lofi::mem::ArrayContainerPolicy<T, Size, Align, lofi::mem::SubAllocPolicy>;
 
  template<typename T>
  using StackPackedArray = lofi::mem::PackedArrayContainerPolicy<T, DefaultStackArraySize, DefaultAlignment, lofi::mem::StackAllocPolicy>;
 
  template<typename T>
  using HeapPackedArray = lofi::mem::PackedArrayContainerPolicy<T, DefaultStackArraySize, DefaultAlignment, lofi::mem::MAllocPolicy>;
 
  template<typename T>
  using PackedArray = lofi::mem::PackedArrayContainerPolicy<T, DefaultStackArraySize, DefaultAlignment, lofi::mem::SubAllocPolicy>;
  
  template<typename T, u64 Size, u64 Align = DefaultAlignment>
  using SizedStackPackedArray = lofi::mem::PackedArrayContainerPolicy<T, Size, Align, lofi::mem::StackAllocPolicy>;
 
  template<typename T, u64 Size, u64 Align = DefaultAlignment>
  using SizedHeapPackedArray = lofi::mem::PackedArrayContainerPolicy<T, Size, Align, lofi::mem::MAllocPolicy>;
 
  template<typename T, u64 Size, u64 Align = DefaultAlignment>
  using SizedPackedArray = lofi::mem::PackedArrayContainerPolicy<T, Size, Align, lofi::mem::SubAllocPolicy>;
  
  template<typename T>
  using StackSparseArray = lofi::mem::SparseArrayContainerPolicy<T, DefaultStackArraySize, DefaultAlignment, lofi::mem::StackAllocPolicy>;
 
  template<typename T>
  using HeapSparseArray = lofi::mem::SparseArrayContainerPolicy<T, DefaultHeapArraySize, DefaultAlignment, lofi::mem::MAllocPolicy>;
 
  template<typename T>
  using SparseArray = lofi::mem::SparseArrayContainerPolicy<T, DefaultArraySize, DefaultAlignment, lofi::mem::SubAllocPolicy>;
 
  template<typename T, size_t Size, size_t Align = DefaultAlignment>
  using SizedStackSparseArray = lofi::mem::SparseArrayContainerPolicy<T, Size, Align, lofi::mem::StackAllocPolicy>;
 
  template<typename T, size_t Size, size_t Align = DefaultAlignment>
  using SizedHeapSparseArray = lofi::mem::SparseArrayContainerPolicy<T, Size, Align, lofi::mem::MAllocPolicy>;
 
  template<typename T, size_t Size, size_t Align = DefaultAlignment>
  using SizedSparseArray = lofi::mem::SparseArrayContainerPolicy<T, Size, Align, lofi::mem::SubAllocPolicy>;
 
  template<typename KeyT, typename T, size_t Size, size_t Align = DefaultAlignment>
  using StackLinearMap = lofi::mem::LinearMap<T, Align, lofi::mem::StackAllocPolicy, KeyT, Size>;

  template<typename KeyT, typename T, size_t Size, size_t Align = DefaultAlignment>
  using HeapLinearMap = lofi::mem::LinearMap<T, Align, lofi::mem::MAllocPolicy, KeyT, Size>;

  template<typename KeyT, typename T>
  using LinearMap = lofi::mem::DynamicMap<T, KeyT>;

  template<typename KeyT, typename T, size_t Align = DefaultAlignment>
  using HashMap = lofi::mem::HashMap<T, Align, lofi::mem::SubAllocPolicy, KeyT, DefaultArraySize>; 

  template<typename KeyT, typename T, size_t Align = DefaultAlignment>
  using StackHashMap = lofi::mem::HashMap<T, Align, lofi::mem::StackAllocPolicy, KeyT, DefaultArraySize>; 

  template<typename T, size_t Size, size_t Align = DefaultAlignment>
  using StackDoubleBuffer = lofi::mem::DoubleBufferContainerPolicy<T, Size, Align, lofi::mem::StackAllocPolicy>;
 
  template<typename T, size_t Size, size_t Align = DefaultAlignment>
  using HeapDoubleBuffer = lofi::mem::DoubleBufferContainerPolicy<T, Size, Align, lofi::mem::MAllocPolicy>;
 
  template<typename T, size_t Size, size_t Align = DefaultAlignment>
  using DoubleBuffer = lofi::mem::DoubleBufferContainerPolicy<T, Size, Align, lofi::mem::SubAllocPolicy>;

  template<typename T, size_t Size, size_t Align = DefaultAlignment>
  using HeapQueue = lofi::mem::PackedRingBufferContainerPolicy<T, Size, Align, lofi::mem::MAllocPolicy>;
  
  template<typename T, size_t Size = DefaultStackArraySize, size_t Align = DefaultAlignment>
  using StackQueue = lofi::mem::PackedRingBufferContainerPolicy<T, Size, Align, lofi::mem::StackAllocPolicy>;
  
  template<typename T, size_t Size = DefaultArraySize, size_t Align = DefaultAlignment>
  using Queue = lofi::mem::PackedRingBufferContainerPolicy<T, Size, Align, lofi::mem::SubAllocPolicy>;
 
}		// -----  end of namespace roxi  ----- 
