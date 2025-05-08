// =====================================================================================
//
//       Filename:  rx_frame_manager.hpp
//
//    Description:  
//
//        Version:  1.0
//        Created:  2024-07-24 4:20:45 PM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Roxi Graves (rg), nada
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#pragma once
#include "rx_thread_pool.hpp"

namespace roxi {
  namespace frame {
    using ID = u64;
  }		// -----  end of namespace frame  ----- 
 
  class FrameManager {
  private:
    Counter _frames[RoxiNumFrames];
  public:

    const u64 get_frame_count(frame::ID id) {
      id &= RoxiNumFrames - 1;
      return _frames[id].get_count();
    }

    void update_frame(frame::ID id) {
      id &= RoxiNumFrames - 1;
      _frames[id]++;
    }

    Counter& get_counter(frame::ID id) {
      id &= RoxiNumFrames - 1;
      return _frames[id];
    }

    void reset_frame(frame::ID id) {
      id &= RoxiNumFrames - 1;
      _frames[id].reset();
    }

  };
}		// -----  end of namespace roxi  ----- 
