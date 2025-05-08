// =====================================================================================
//
//       Filename:  sum.hpp
//
//    Description:  
//
//        Version:  1.0
//        Created:  2024-02-10 3:16:11 PM
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
   
    template<int Acc, int... Is>
    struct sum : Int<Acc> {};

    template<int Acc, int I, int... Is>
    struct sum<Acc, I, Is...> : sum<Acc + I, Is...> {};


  }		// -----  end of namespace meta  ----- 

}		// -----  end of namespace lofi  ----- 
