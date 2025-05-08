// =====================================================================================
//
//       Filename:  rx_vocab.h
//
//    Description:  type definitions for roxi 
//
//        Version:  1.0
//        Created:  2024-04-28 9:36:50 AM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Roxi Graves (rg), nada
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#pragma once
#include "pch.h"
#include "../../../lofi/core/include/l_vocab.hpp"

#define RX_END() return true;
#define RX_END_RESULT(result) return (result);

namespace roxi {

  static constexpr u32 RoxiECSConfigID         =        0;
  static constexpr u64 default_num_threads     =        4;
  static constexpr u64 RoxiNumFibers           =      128;
  static constexpr u64 RoxiNumFrames           =        4;
  static constexpr u64 DefaultStackArraySize   =       64;
  static constexpr u64 DefaultHeapArraySize    =      256;
  static constexpr u64 DefaultArraySize        =  MAX_u32 - 1;

  static constexpr u64 DefaultAlignment        =        8;
  static constexpr u64 RoxiNumThreads          =        4;

  template<typename... Ts>
  using List = lofi::List<Ts...>;
}		// -----  end of namespace roxi  ----- 

