// =====================================================================================
//
//       Filename:  l_parser.hpp
//
//    Description:  template parser experiment 
//
//        Version:  1.0
//        Created:  2024-11-25 8:53:39 AM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Roxi Graves (rg), nada
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#pragma once
#include "l_memory.hpp"
#include "l_variant.hpp"
#include "l_string.hpp"
#include "l_file.hpp"


namespace lofi {

  template<typename... Ts>
  using Token = variant<Ts...>;

  template<u64 MaxNumEntries, typename... TokenTypes>
  class Lexer {
  private:
    struct Entry {
      u32 offset : 30;
      u32 type : 2;
    };
    mem::ArrayContainerPolicy<Entry, MaxNumEntries, 8, mem::SubAllocPolicy> entries;
    Arena<> data;

  public:

  };

}		// -----  end of namespace lofi  ----- 
