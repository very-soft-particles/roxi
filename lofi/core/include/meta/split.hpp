// =====================================================================================
//
//       Filename:  split.hpp
//
//    Description:  
//
//        Version:  1.0
//        Created:  2024-02-10 3:18:23 PM
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
   
    template<typename TOut, typename TCur, typename TDelim, typename... Ts>
    struct split;

    template<typename... Os, typename... Cs, typename TDelim, typename T, typename... Ts> // next is not delim 
    struct split<List<Os...>, List<Cs...>, TDelim, T, Ts...>                              // more left
    : split<List<Os...>, List<Cs..., T>, TDelim, Ts...> {};                               

    template<typename... Os, typename... Cs, typename TDelim, typename... Ts>             // next is delim
    struct split<List<Os...>, List<Cs...>, TDelim, TDelim, Ts...>                         // more left
    : split<List<Os..., List<Cs...>>, List<>, TDelim, Ts...> {};                           

    template<typename... Os, typename... Cs, typename TDelim, typename T>                 // next is not delim
    struct split<List<Os...>, List<Cs...>, TDelim, T> {                                   // no more left
      using type = List<Os..., List<Cs...,T>>;
    };                           

    template<typename... Os, typename... Cs, typename TDelim>                             // next is delim
    struct split<List<Os...>, List<Cs...>, TDelim, TDelim> {                              // no more left
      using type = List<Os..., List<Cs...>>;
    };                           

    // same but with empty current list
    template<typename... Os, typename TDelim, typename... Ts>                             // next is delim
    struct split<List<Os...>, List<>, TDelim, TDelim, Ts...>                              // more left
    : split<List<Os...>, List<>, TDelim, Ts...> {};                           

    template<typename... Os, typename TDelim>                                             // next is delim
    struct split<List<Os...>, List<>, TDelim, TDelim> {                                   // no more left
      using type = List<Os...>;
    };
  }		// -----  end of namespace meta  ----- 
}		// -----  end of namespace lofi  ----- 
