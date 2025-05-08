// =====================================================================================
//
//       Filename:  rx_math.hpp
//
//    Description:  
//
//        Version:  1.0
//        Created:  2024-06-26 8:20:15 PM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Roxi Graves (rg), nada
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#pragma once
#include "rx_tuple.hpp"

namespace roxi {

  template<u8 Dimension, typename T = u32>
  using Vector = typename lofi::meta::repeat<T, Tuple, Dimension>::type;

  template<typename T>
  using Vertex = Tuple<Vector<3, T>, Vector<3, T>, Vector<2, T>>;

  using VertexIndex = typename lofi::meta::repeat<u64, Tuple, 3>::type;

  namespace math {

    template<typename T = u32>
    Vertex<T> create_vertex(Vector<3, T> position, Vector<3, T> normal, Vector<2, T> texture_coord) {
      Vertex<T> vertex;
      vertex.template get<0>() = position;
      vertex.template get<1>() = normal;
      vertex.template get<2>() = texture_coord;
      return vertex;
    }

    VertexIndex create_vertex_index(u32 position_index, u32 normal_index, u32 texture_coord_index);

  }		// -----  end of namespace math  ----- 

}		// -----  end of namespace roxi  ----- 
