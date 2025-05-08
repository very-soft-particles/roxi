// =====================================================================================
//
//       Filename:  at.hpp
//
//    Description:  
//
//        Version:  1.0
//        Created:  2024-02-10 3:20:11 PM
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

    template<typename I, typename C = identity>
    struct at {};

    namespace detail {

      template<u32 N, typename I, typename C>
      struct dispatch<N, at<I, C>> : dispatch<N, drop<I, C>> {};
      
      template<u32 N, typename C>
      struct dispatch<N, at<Int<0>, C>> {                                 
        template<typename T0, typename... Ts>                     
          using f = typename dispatch<N, C>::template f<T0>;        
      };


      template<u32 N, typename C>
      struct dispatch<N, at<Int<1>, C>> {
        template<typename T0, typename T1, typename... Ts>
          using f = typename dispatch<N, C>::template f<T1>;
      };

      template<u32 N, typename C>
      struct dispatch<N, at<Int<2>, C>> {                                 
        template<typename T0, typename T1, typename T2, typename... Ts>                     
          using f = typename dispatch<N, C>::template f<T2>;        
      };

      template<u32 N, typename C>
      struct dispatch<N, at<Int<3>, C>> {
        template<typename T0, typename T1, typename T2, typename T3, typename... Ts>
          using f = typename dispatch<N, C>::template f<T3>;
      };

      template<u32 N, typename C>
      struct dispatch<N, at<Int<4>, C>> {                                 
        template<typename T0, typename T1, typename T2, typename T3, typename T4, typename... Ts>                     
          using f = typename dispatch<N, C>::template f<T4>;       
      };


      template<u32 N, typename C>
      struct dispatch<N, at<Int<5>, C>> {
        template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename... Ts>
          using f = typename dispatch<N, C>::template f<T5>;
      };

      template<u32 N, typename C>
      struct dispatch<N, at<Int<6>, C>> {                                 
        template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename... Ts>
          using f = typename dispatch<N, C>::template f<T6>;
      };

      template<u32 N, typename C>
      struct dispatch<N, at<Int<7>, C>> {
        template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename... Ts>
          using f = typename dispatch<N, C>::template f<T7>;
      };

    }		// -----  end of namespace detail  ----- 
  }		// -----  end of namespace meta  ----- 
}		// -----  end of namespace lofi  ----- 
