// =====================================================================================
//
//       Filename:  join.hpp
//
//    Description:  
//
//        Version:  1.0
//        Created:  2024-02-10 3:17:17 PM
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

    template<typename TOut, typename TDelim, typename... Ts>
    struct join {
      static_assert(AlwaysFalse<TOut>::value, "incorrect parameter format in join, expected a list of lists");
    };

    template<typename... Os, typename TDelim, typename... Ls, typename... Ts>
    struct join<List<Os...>, TDelim, List<Ls...>, Ts...> : join<List<Os...,TDelim,Ls...>,TDelim, Ts...> {};

    template<typename... Os, typename TDelim>
    struct join<List<Os...>, TDelim> : Return<List<Os...>> {};

  }		// -----  end of namespace meta  ----- 

}		// -----  end of namespace lofi  ----- 
