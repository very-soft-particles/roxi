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
  
    template<class T, class Tuple>
    struct has_type;
  
    template<class T, class... Types>
    struct has_type<T, List<T, Types...>> {
      static constexpr bool value = true;
    };
  
    template<class T, class U, class... Types>
    struct has_type<T, List<U, Types...>> {
      static constexpr bool value = has_type<T, List<Types...>>::value;
    };
  
    template<class T>
    struct has_type<T, List<>> {
      static constexpr bool value = false;
    };
  }		// -----  end of namespace meta  ----- 
}		// -----  end of namespace lofi  ----- 
