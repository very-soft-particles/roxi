// =====================================================================================
//
//       Filename:  rx_arena.cpp
//
//    Description:  
//
//        Version:  1.0
//        Created:  2025-02-15 11:51:52 AM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Robyn Gray (rg), ...
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#include "rx_arena.hpp"

template class lofi::Arena<roxi::DefaultArraySize, roxi::DefaultAlignment, lofi::mem::SubAllocPolicy>;
