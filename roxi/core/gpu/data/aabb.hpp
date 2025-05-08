// =====================================================================================
//
//       Filename:  aabb.hpp
//
//    Description:  aabb data 
//
//        Version:  1.0
//        Created:  2024-10-10 8:25:28 AM
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
 
  struct AABB {
   	alignas(16)
	  glm::vec4 minPoint;
	  alignas(16)
	  glm::vec4 maxPoint;
  };

}		// -----  end of namespace roxi  ----- 
