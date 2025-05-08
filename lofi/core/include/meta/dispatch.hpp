// =====================================================================================
//
//       Filename:  dispatch.hpp
//
//    Description:  
//
//        Version:  1.0
//        Created:  2024-02-11 4:38:30 PM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Roxi Graves (rg), nada
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#pragma once

#include "../l_meta.hpp"

// from (boost).tmp

namespace lofi {
  namespace meta {
    namespace detail {
      constexpr unsigned find_dispatch(unsigned n) {
        return n <= 8 ? n :
          n < 16 ?
          9 :
          n == 16 ?
          16 :
          n < 32 ?
          17 :
          n == 32 ?
          32 :
          n < 64 ? 33 : n == 64 ? 64 : n < 128 ? 65 : n == 128 ? 128 : 129;
      }

      template <unsigned N, typename T>
      struct dispatch;

      template <typename C>
      struct dispatch_unknown {
        template <typename... Ts>
          using f = typename dispatch<find_dispatch(sizeof...(Ts)), C>::template f<Ts...>;
      };
    }		// -----  end of namespace detail  ----- 
  }		// -----  end of namespace meta  ----- 
}		// -----  end of namespace lofi  ----- 
