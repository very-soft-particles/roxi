// =====================================================================================
//
//       Filename:  message_handler.h
//
//    Description:  
//
//        Version:  1.0
//        Created:  2024-05-08 9:08:51 AM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Roxi Graves (rg), nada
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#pragma once
#include "rx_input.h"
#include "rx_container.hpp"
#include "rx_log.hpp"
#include <bitset>

#define HANDLE_MSG MessageHandler::instance()->handle_msg
#define READ_MSG MessageHandler::instance()->swap_and_get_messages
#define PROCESS MessageHandler::instance()->process_messages

namespace roxi {

  namespace win {

    class MessageHandler {
    private:
      static constexpr u32 NumKeys = 256;
      static constexpr u32 NumMouseButtons = 4;
      using Message = input::Message;
      using InputBuffer = DoubleBuffer<Message, KB(4)>;
      using index_t = typename InputBuffer::index_t;
      InputBuffer input_msg_buffer{};
      struct {
        std::bitset<NumKeys> keyboard = 0;
        std::bitset<NumMouseButtons> mouse = 0;
        u32 msg_time = 0;
      //  u32 last_msg_time = 0;
      } state;
      u32 start_time = 0;
      i64 internal_clock_start = 0;
    public:
      MessageHandler();
      b8 process_messages();
      static MessageHandler* instance();
      LRESULT handle_msg(HWND h_wnd, UINT msg, WPARAM w_param, LPARAM l_param, u32 width, u32 height) noexcept;
      Message* swap_and_get_messages(u32* count);
    };

  }		// -----  end of namespace win  ----- 

}		// -----  end of namespace roxi  ----- 
