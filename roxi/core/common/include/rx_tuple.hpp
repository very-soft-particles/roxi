// =====================================================================================
//
//       Filename:  rx_tuple.hpp
//
//    Description:  
//
//        Version:  1.0
//        Created:  2025-02-15 11:39:59 AM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Robyn Gray (rg), ...
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#pragma once
#include "../../../lofi/core/include/l_tuple.hpp"

namespace roxi {
  template<typename... Ts>
  using Tuple = lofi::tuple<Ts...>;

}		// -----  end of namespace roxi  ----- 

