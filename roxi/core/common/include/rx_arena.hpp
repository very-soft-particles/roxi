// =====================================================================================
//
//       Filename:  rx_arena.hpp
//
//    Description:  
//
//        Version:  1.0
//        Created:  2025-02-15 11:48:34 AM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Robyn Gray (rg), ...
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#pragma once
#include "../../../lofi/core/include/l_arena.hpp"
#include "rx_vocab.h"


namespace roxi {
 
  using StackArena = lofi::Arena<KB(4), DefaultAlignment, lofi::mem::StackAllocPolicy>;

  template<u64 Size, u64 Align = DefaultAlignment>
  using SizedStackArena = lofi::Arena<Size, Align, lofi::mem::StackAllocPolicy>;

  template<size_t InitialCap, size_t Alignment>
  using HeapArena = lofi::Arena<InitialCap, Alignment, lofi::mem::MAllocPolicy>;

  using Arena = lofi::Arena<DefaultArraySize, DefaultAlignment, lofi::mem::SubAllocPolicy>;

}		// -----  end of namespace roxi  ----- 

extern template class lofi::Arena<roxi::DefaultArraySize, roxi::DefaultAlignment, lofi::mem::SubAllocPolicy>;
