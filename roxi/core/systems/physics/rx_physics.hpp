// =====================================================================================
//
//       Filename:  rx_physics.hpp
//
//    Description:  
//
//        Version:  1.0
//        Created:  2025-02-13 5:46:09 AM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Robyn Gray (rg), ...
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#pragma once
#include "rx_allocator.hpp"
#include "rx_system.hpp"



namespace roxi {
  namespace systems {
    namespace physics {

      DEFINE_JOB(init) {
        return true;
      }

      DEFINE_JOB(update) {

        const u32 frame_count = GB(1);
        return true;
      }

      DEFINE_JOB(terminate) {
        return true;
      }
 
    }		// -----  end of namespace physics  ----- 

  }		// -----  end of namespace systems  ----- 
  RX_DECLARE_SYSTEM(physics);
}		// -----  end of namespace roxi  ----- 

