// =====================================================================================
//
//       Filename:  rx_allocator.cpp
//
//    Description:  
//
//        Version:  1.0
//        Created:  2025-02-15 11:53:05 AM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Robyn Gray (rg), ...
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#include "rx_allocator.hpp"

extern template class lofi::LockFreeRuntimeAllocator<roxi::LockFreeAllocatorPoolID>;
