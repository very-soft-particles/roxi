// =====================================================================================
//
//       Filename:  l_poly.hpp
//
//    Description:  
//
//        Version:  1.0
//        Created:  2024-02-21 5:14:04 PM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Roxi Graves (rg), nada
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#pragma once
#include "l_variant.hpp"



namespace lofi {

  template<typename BaseT>
  class poly {
    using v_table_t = typename BaseT::v_table;
    using interface_t = typename BaseT::interface_t;

    void* _data;
    v_table_t* v_table;
    interface_t interface;
  public:
    template <typename Any>
    poly(Any* derived) : _data{(void*)derived}, v_table{&BaseT::template v_table_for<Any>}, interface(v_table) 
    {}

    interface_t* operator()() {
      return &interface;
    }

  };

}		// -----  end of namespace lofi  ----- 
