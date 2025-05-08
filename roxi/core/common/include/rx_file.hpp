// =====================================================================================
//
//       Filename:  rx_file.hpp
//
//    Description: 
//
//        Version:  1.0
//        Created:  2025-04-14 8:14:59 PM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Robyn Gray (rg), ...
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#pragma once
#include "../../../lofi/core/include/l_file.hpp"


namespace roxi {
 
  using File = lofi::File;
  using FileType = lofi::FileType;
  static constexpr char NewLine = lofi::NewLine;
  static constexpr char Tab = lofi::Tab;
  static constexpr char LineReturn = lofi::LineReturn;
  static constexpr char Null = lofi::Null;

}		// -----  end of namespace roxi  ----- 
