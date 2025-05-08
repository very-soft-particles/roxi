// =====================================================================================
//
//       Filename:  vertex.hpp
//
//    Description:  vertex data 
//
//        Version:  1.0
//        Created:  2024-10-10 8:10:15 AM
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
  
  struct Vertex {
    glm::vec3 position;
    f32 uv_x;
    glm::vec3 normal;
    f32 uv_y;
    glm::vec4 colour;
    Vertex(glm::vec3&& pos, glm::vec3&& norm, glm::vec4&& col, f32 n_uv_x, f32 n_uv_y)
      : position(pos), uv_x(n_uv_x), normal(norm), uv_y(n_uv_y), colour(col) {} // maybe not?
  };

}		// -----  end of namespace roxi  ----- 
