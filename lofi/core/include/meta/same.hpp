// =====================================================================================
//
//       Filename:  at.hpp
//
//    Description:  
//
//        Version:  1.0
//        Created:  2024-02-10 3:20:11 PM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Roxi Graves (rg), nada
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#pragma once
#include "../l_meta.hpp"


namespace lofi {
  namespace meta {
 
    template<typename T, typename U>
    struct is_same {
      using type = FalseType;
    };
  
    template<typename T>
    struct is_same<T, T> {
      using type = TrueType;
    };
 
  }		// -----  end of namespace meta  ----- 
}		// -----  end of namespace lofi  ----- 
