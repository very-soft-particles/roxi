// =====================================================================================
//
//       Filename:  rx_entity.hpp
//
//    Description:  entity id structs 
//
//        Version:  1.0
//        Created:  2024-06-25 2:06:46 PM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Roxi Graves (rg), nada
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#pragma once
#include "../l_vocab.hpp"

namespace lofi {
  namespace ecs {

    static constexpr u32 MAX_ENTITIES = MAX_u16;

    struct Entity {
      Entity(u16 i, u16 gen) : index{i}, generation{gen} {}
      Entity() : index{MAX_u16}, generation{MAX_u16} {}
      u16 index;
      u16 generation;
    };

    struct Handle {
      Handle(u16 i, u16 gen) : index{i}, generation{gen} {}
      u16 index;
      u16 generation;
    };

  }		// -----  end of namespace ecs  ----- 
}		// -----  end of namespace lofi  ----- 
