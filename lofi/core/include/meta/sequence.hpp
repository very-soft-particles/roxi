// =====================================================================================
//
//       Filename:  size.hpp
//
//    Description:  
//
//        Version:  1.0
//        Created:  2024-02-10 3:22:33 PM
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
  
    template<int N, typename... Is>
    struct int_sequence : int_sequence<N-1, Int<N-1>, Is...> {};
  
    template<typename... Is>
    struct int_sequence<0, Is...> {
      using type = List<Int<0>,Is...>;
    };
  }		// -----  end of namespace meta  ----- 
}		// -----  end of namespace lofi  ----- 
