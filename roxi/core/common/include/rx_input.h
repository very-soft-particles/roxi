// =====================================================================================
//
//       Filename:  rx_input.h
//
//    Description:  
//
//        Version:  1.0
//        Created:  2024-05-08 1:53:16 PM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Roxi Graves (rg), nada
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#pragma once
#include "rx_vocab.h"
#include "rx_time.h"

namespace roxi {
  static constexpr u8 EscapeKey = 0x1b;
  static constexpr u8 SpaceKey = 0x20;

  namespace input {
    struct Position {
      i16 x = MAX_u16;
      i16 y = MAX_u16;
      operator u32() {
        u32 value = x;
        value <<= 16;
        value |= y;
        return value;
      }
      Position operator=(u32 value) {
        Position p;
        p.y = value;
        p.x = (value >> 16);
        return p;
      }
    };
   
    struct Message {
      enum class Event {
        KeyPressed,
        KeyReleased,
        MouseLeftPressed,
        MouseLeftReleased,
        MouseRightPressed,
        MouseRightReleased,
        MouseMiddlePressed,
        MouseMiddleReleased,
        MouseWheelDelta,
        MouseMoved,
        Quit,
        NumEvents
      };
      u64 time = MAX_u64;
      u32 code = MAX_u32;
      Event event = Event::NumEvents;
    };

  }		// -----  end of namespace input  ----- 

}		// -----  end of namespace roxi  ----- 
