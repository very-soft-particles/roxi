// =====================================================================================
//
//       Filename:  Window.h
//
//    Description:  
//
//        Version:  1.0
//        Created:  2024-05-06 5:20:56 PM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Roxi Graves (rg), nada
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#pragma once
#include "message_handler.h"

namespace roxi {
 
  namespace win {

    class Window {
    private:
      class WindowClass {
      public:
        static const char* get_name() noexcept;
        static HINSTANCE get_instance() noexcept;

      private:
        WindowClass() noexcept;
        ~WindowClass();
        static WindowClass window_class;
        static const char* win_name;
        HINSTANCE h_instance{};

      };

    	void confine_cursor() noexcept;
    	void free_cursor() noexcept;
    	void show_cursor() noexcept;
    	void hide_cursor() noexcept;
    	void enable_imgui_mouse() noexcept;
    	void disable_imgui_mouse() noexcept;
      static LRESULT CALLBACK handle_msg_setup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;
	    static LRESULT CALLBACK handle_msg_thunk(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;
    public:
      static struct Extent {
        u32 width = 0;
        u32 height = 0;
        operator VkExtent2D();
      } extents;
      b8 init(u32 width, u32 height, const char* name);
      b8 terminate();

      HINSTANCE get_instance() noexcept;
      HWND get_window() noexcept;

      Extent get_extents() const noexcept; 

      b8 enable_cursor() noexcept;
      b8 disable_cursor() noexcept;
    private:

      HWND h_wnd;
    };
    

  }		// -----  end of namespace win  ----- 
   
}		// -----  end of namespace roxi  ----- 
