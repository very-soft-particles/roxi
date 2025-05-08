// =====================================================================================
//
//       Filename:  rx_math.cpp
//
//    Description:  
//
//        Version:  1.0
//        Created:  2024-06-26 8:20:32 PM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Roxi Graves (rg), nada
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#include "rx_math.hpp"


namespace roxi {
  
  namespace math {
   
    VertexIndex create_vertex_index(u32 position_index, u32 normal_index, u32 texture_coord_index) {
      VertexIndex vertex_index;
      vertex_index.template get<0>() = position_index;
      vertex_index.template get<1>() = normal_index;
      vertex_index.template get<2>() = texture_coord_index;
      return vertex_index;
    }

  }		// -----  end of namespace math  ----- 

}		// -----  end of namespace roxi  ----- 
