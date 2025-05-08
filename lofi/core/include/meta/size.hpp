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
    template<typename ListT>
    struct list_size;
    template<typename... Ts>
    struct list_size<List<Ts...>> {
      static constexpr size_t value = sizeof...(Ts);
    };
  }		// -----  end of namespace meta  ----- 
}		// -----  end of namespace lofi  ----- 
