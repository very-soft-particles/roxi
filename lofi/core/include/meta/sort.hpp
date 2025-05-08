// =====================================================================================
//
//       Filename:  sort.hpp
//
//    Description:  
//
//        Version:  1.0
//        Created:  2024-02-10 3:21:23 PM
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
    template
      < typename TOut
      , template<typename, typename> class TPred
      , typename TInsert, bool BTag, typename... Ts>
    struct sort_insert;

    template
      < typename... Os
      , template<typename, typename> class TPred
      , typename TInsert, typename T1, typename T2, typename... Ts>
    struct sort_insert<List<Os...>, TPred, TInsert, true, T1, T2, Ts...>
      : sort_insert<List<Os..., T1>, TPred, TInsert, TPred<T2, TInsert>::value, T2, Ts...> {};

    template
      < typename... Os
      , template<typename, typename> class TPred
      , typename TInsert, typename... Ts>
    struct sort_insert<List<Os...>, TPred, TInsert, true, Ts...> {
      using type = List<Os..., Ts..., TInsert>;
    };

    template
      < typename... Os
      , template<typename, typename> class TPred
      , typename TInsert, typename... Ts>
    struct sort_insert<List<Os...>, TPred, TInsert, false, Ts...> {
      using type = List<Os..., TInsert, Ts...>;
    };

    template<typename TOut, template<typename, typename> class P, typename... Ts>
    struct sort {
      static_assert(AlwaysFalse<TOut>::value, "implausible parameters");
    };
  
    template<typename O, typename... Os, template<typename, typename> class TPred, typename TInsert, typename... Ts>
    struct sort<List<O, Os...>, TPred, TInsert, Ts...> : sort<typename meta::sort_insert<List<>, TPred, TInsert, TPred<O, TInsert>::value, O, Os...>::type, TPred, Ts...> {};
  
    template<typename... Os, template<typename, typename> class TPred, typename TInsert, typename... Ts>
    struct sort<List<Os...>, TPred, TInsert, Ts...> : sort<typename meta::sort_insert<List<>, TPred, TInsert, false, Os...>::type, TPred, Ts...> {};
  
    template<typename... Os, template<typename, typename> class TPred, typename... Ts>
    struct sort<List<Os...>, TPred, Ts...>  {
      using type = List<Os...>;
    };
  }		// -----  end of namespace meta  ----- 
}		// -----  end of namespace lofi  ----- 
