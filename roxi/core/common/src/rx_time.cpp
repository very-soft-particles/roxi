// =====================================================================================
//
//       Filename:  rx_time.cpp
//
//    Description:  
//
//        Version:  1.0
//        Created:  2024-04-30 4:55:16 PM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Roxi Graves (rg), nada
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#include "../include/rx_time.h"


  // Taken from the Rust code base: https://github.com/rust-lang/rust/blob/3809bbf47c8557bd149b3e52ceb47434ca8378d5/src/libstd/sys_common/mod.rs#L124
  // Computes (value*numer)/denom without overflow, as long as both
  // (numer*denom) and the overall result fit into i64 (which is the case
  // for our time conversions).
  static i64 int64_mul_div( i64 value, i64 numer, i64 denom ) {
    const i64 q = value / denom;
    const i64 r = value % denom;
    // Decompose value as (value/denom*denom + value%denom),
    // substitute into (value*numer)/denom and simplify.
    // r < denom, so (denom*numer) is the upper bound of (r*numer)
    return q * numer + r * numer / denom;
  }


namespace roxi {
 
#if COMPILER_CL
    static LARGE_INTEGER Time::s_frequency{};
#else
    clock_t Time::s_frequency = 0;
#endif

}		// -----  end of namespace roxi  ----- 
