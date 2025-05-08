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
 
    template<class T, class ListT>
    struct list_index;
  
    template<class T, class... Types>
    struct list_index<T, List<T, Types...>> {
      static constexpr size_t value = 0;
    };
  
    template<class T, class U, class... Types>
    struct list_index<T, List<U, Types...>> {
      static constexpr size_t value = 1 + list_index<T, List<Types...>>::value;
    };

  }		// -----  end of namespace meta  ----- 
}		// -----  end of namespace lofi  ----- 
