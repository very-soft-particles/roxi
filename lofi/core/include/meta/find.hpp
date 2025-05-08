// =====================================================================================
//
//       Filename:  find.hpp
//
//    Description:  
//
//        Version:  1.0
//        Created:  2024-02-10 3:10:33 PM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Roxi Graves (rg), nada
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#pragma once
#include "drop.hpp"


namespace lofi {
  namespace meta {
    
    template<typename T, typename C = identity>
    struct find {};

    template<class TFind>
    struct find<TFind> {

      template<typename... Ts>
      using f = typename C::f template<Ts...>;

    };

    template<int I, bool Found, template<typename...> class Pred, typename... Ts>
    struct pred_find : Int<-1> {};

    template<int I, template<typename...> class Pred, typename... Ts>
    struct pred_find<I, true, Pred, Ts...> : Int<I> {};

    template<int I, template<typename...> class Pred, typename T, typename... Ts>
    struct pred_find<I, false, Pred, T, Ts...> : pred_find<I + 1, Pred<T>::value, Pred, Ts...> {};

  }		// -----  end of namespace meta  ----- 
}		// -----  end of namespace lofi  ----- 
