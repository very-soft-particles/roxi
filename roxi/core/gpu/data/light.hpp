// =====================================================================================
//
//       Filename:  light.hpp
//
//    Description:  light data 
//
//        Version:  1.0
//        Created:  2024-10-10 8:24:10 AM
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
 
  struct Light {
	  alignas(16)
	  glm::vec4 position;
	  alignas(16)
	  glm::vec4 colour = glm::vec4(1.0f);
	  alignas(4)
	  f32 radius = 1.0f;
  };

  struct LightCell {
	  alignas(4)
	  u32 offset;
	  alignas(4)
	  u32 count;
  };

}		// -----  end of namespace roxi  ----- 
