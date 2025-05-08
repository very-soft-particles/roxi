// =====================================================================================
//
//       Filename:  bda.hpp
//
//    Description:  buffer device address
//
//        Version:  1.0
//        Created:  2024-10-10 7:59:20 AM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Roxi Graves (rg), nada
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#pragma once
#include "rx_vocab.h"

namespace roxi {
  namespace gpu {
    
    struct BDA {
      u64 vertex_buffer_address;
      u64 index_buffer_address;
      u64 mesh_data_buffer_address; // Per-mesh material
      u64 _; // pad
    };

  }		// -----  end of namespace gpu  ----- 
}		// -----  end of namespace roxi  ----- 
