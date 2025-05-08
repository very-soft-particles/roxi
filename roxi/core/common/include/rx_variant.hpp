// =====================================================================================
//
//       Filename:  rx_variant.hpp
//
//    Description:  
//
//        Version:  1.0
//        Created:  2025-02-15 11:40:52 AM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Robyn Gray (rg), ...
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#pragma once
#include "../../../lofi/core/include/l_variant.hpp"

namespace roxi {

  template<typename... Ts>
  using Variant = lofi::variant<Ts...>;

}		// -----  end of namespace roxi  ----- 
