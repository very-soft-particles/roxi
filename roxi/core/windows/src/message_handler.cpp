// =====================================================================================
//
//       Filename:  message_handler.cpp
//
//    Description:  
//
//        Version:  1.0
//        Created:  2024-05-08 9:12:48 AM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Roxi Graves (rg), nada
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#include "message_handler.h"
#include "rx_input.h"
#include "rx_allocator.hpp"
#include <winuser.h>


namespace roxi {
 
  namespace win {

    MessageHandler s_message_handler;

    MessageHandler::MessageHandler() : input_msg_buffer(ALLOCATE(sizeof(Message) * KB(4))) {};

    MessageHandler* MessageHandler::instance() {
      return &s_message_handler;
    }

    b8 MessageHandler::process_messages() {
     	MSG msg;
	// while queue has messages, remove and dispatch them (but do not block on empty queue)
	    while( PeekMessage( &msg,nullptr,0,0,PM_REMOVE ) )
	    {
	    	// check for quit because peekmessage does not signal this via return val
	    	if( msg.message == WM_QUIT )
	    	{
	    		// return optional wrapping int (arg to PostQuitMessage is in wparam) signals quit
	    		return false;
	    	}

	    	// TranslateMessage will post auxilliary WM_CHAR messages from key msgs

        if(start_time == 0) [[unlikely]] {
          start_time = msg.time;
        }
        //state.last_msg_time = state.msg_time;     // for tracking time deltas
        state.msg_time = (u32)msg.time - start_time;
        //if(state.last_msg_time > state.msg_time) // handle the case where the timer wraps


	    	TranslateMessage( &msg );
	    	DispatchMessage( &msg );
	    }
      return true;
    }

    LRESULT MessageHandler::handle_msg(HWND h_wnd, UINT msg, WPARAM w_param, LPARAM l_param, u32 width, u32 height) noexcept {
      switch(msg) {
        case WM_CLOSE:
          PostQuitMessage(-1);
          return 0;
        case WM_KILLFOCUS:
	      	state.keyboard.reset();
	      	break;
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        {
          if(state.keyboard[w_param] == true)
            break;
          Message* message = input_msg_buffer.write();
          message->time = state.msg_time;
          state.keyboard[w_param] = true;
          message->event = Message::Event::KeyPressed;
          message->code = w_param;
          break;
        }
        case WM_KEYUP:
        case WM_SYSKEYUP:
        {
          if(state.keyboard[w_param] == false)
            break;
          Message* message = input_msg_buffer.write();
          message->time = state.msg_time;
          state.keyboard[w_param] = false;
          message->event = Message::Event::KeyReleased;
          message->code = w_param;
          break;
        }
        case WM_MOUSEMOVE:
        {
          Message* message = input_msg_buffer.write();
          message->time = state.msg_time;
          const POINTS points = MAKEPOINTS(l_param);
          message->event = Message::Event::MouseMoved;
          message->code = (u32)input::Position{points.x, points.y};
          break;
        }
        case WM_LBUTTONDOWN:
        {
          if(state.mouse[0] == true) 
            break;
          Message* message = input_msg_buffer.write();
          message->time = state.msg_time;
          const POINTS points = MAKEPOINTS(l_param);
          if(points.x < 0 || points.x >= width || points.y < 0 || points.y >= height) {
            SetForegroundWindow(h_wnd);
            SetCapture(h_wnd);
          }
          state.mouse[0] = true;
          message->event = Message::Event::MouseLeftPressed;
          message->code = (u32)input::Position{points.x, points.y};
          break;
        }
        case WM_RBUTTONDOWN:
        {
          if(state.mouse[1] == true) 
            break;
          Message* message = input_msg_buffer.write();
          message->time = state.msg_time;
          const POINTS points = MAKEPOINTS(l_param);
          if(points.x < 0 || points.x >= width || points.y < 0 || points.y >= height) {
            SetCapture(h_wnd);
          }
          state.mouse[1] = true;
          message->event = Message::Event::MouseRightPressed;
          message->code = (u32)input::Position{points.x, points.y};
          break;
        }
        case WM_MBUTTONDOWN:
        {
          if(state.mouse[2] == true) 
            break;
          Message* message = input_msg_buffer.write();
          message->time = state.msg_time;
          const POINTS points = MAKEPOINTS(l_param);
          if(points.x < 0 || points.x >= width || points.y < 0 || points.y >= height) {
            SetCapture(h_wnd);
          }
          state.mouse[2] = true;
          message->event = Message::Event::MouseMiddlePressed;
          message->code = (u32)input::Position{points.x, points.y};
          break;
        }
        case WM_LBUTTONUP:
        {
          if(state.mouse[0] == false)
            break;
          Message* message = input_msg_buffer.write();
          message->time = state.msg_time;
          const POINTS points = MAKEPOINTS(l_param);
          if(points.x < 0 || points.x >= width || points.y < 0 || points.y >= height) {
            ReleaseCapture();
          }
          state.mouse[0] = false;
          message->event = Message::Event::MouseLeftReleased;
          message->code = (u32)input::Position{points.x, points.y};
          break;
        }
        case WM_RBUTTONUP:
        {
          if(state.mouse[1] == false)
            break;
          Message* message = input_msg_buffer.write();
          message->time = state.msg_time;
          const POINTS points = MAKEPOINTS(l_param);
          if(points.x < 0 || points.x >= width || points.y < 0 || points.y >= height) {
            ReleaseCapture();
          }
          state.mouse[1] = false;
          message->event = Message::Event::MouseRightReleased;
          message->code = (u32)input::Position{points.x, points.y};
          break;
        }
        case WM_MBUTTONUP:
        {
          if(state.mouse[2] == false)
            break;
          Message* message = input_msg_buffer.write();
          message->time = state.msg_time;
          const POINTS points = MAKEPOINTS(l_param);
          if(points.x < 0 || points.x >= width || points.y < 0 || points.y >= height) {
            ReleaseCapture();
          }
          state.mouse[2] = false;
          message->event = Message::Event::MouseMiddleReleased;
          message->code = (u32)input::Position{points.x, points.y};
          break;
        }
        case WM_MOUSEWHEEL:
        {
          Message* message = input_msg_buffer.write();
          message->time = state.msg_time;
          const int wheel = GET_WHEEL_DELTA_WPARAM(w_param);
          message->event = Message::Event::MouseMiddleReleased;
          message->code = (u32)wheel;
          break;
        }
        default:
        break;
      }
      return DefWindowProc(h_wnd, msg, w_param, l_param);
    }

    MessageHandler::Message* MessageHandler::swap_and_get_messages(u32* count) {
      input_msg_buffer.clear_reads();
      input_msg_buffer.swap_buffers();
      return input_msg_buffer.read((index_t*)count);
    }

  }		// -----  end of namespace win  ----- 

}		// -----  end of namespace roxi  ----- 
