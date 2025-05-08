// =====================================================================================
//
//       Filename:  window.cpp
//
//    Description:  
//
//        Version:  1.0
//        Created:  2024-05-07 3:59:20 PM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Roxi Graves (rg), nada
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#include "../include/window.h"
#include "../../resource/resource.h"
#include <winuser.h>


namespace roxi {
 
  namespace win {

    Window::WindowClass Window::WindowClass::window_class;
    const char* Window::WindowClass::win_name{"roxi"};
    Window::Extent Window::extents;

    Window::WindowClass::WindowClass() noexcept {
      h_instance = GetModuleHandle(nullptr);
      WNDCLASSEX wc = {};
      wc.cbSize = sizeof( wc );
      wc.style = CS_OWNDC;
      wc.lpfnWndProc = handle_msg_setup;
      wc.cbClsExtra = 0;
      wc.cbWndExtra = 0;
      wc.hInstance = get_instance();
      wc.hIcon = (HICON)LoadImage(get_instance(), MAKEINTRESOURCE(IDI_ROXIICON), IMAGE_ICON, 32, 32, 0);
      wc.hCursor = nullptr;
      wc.hbrBackground = nullptr;
      wc.lpszMenuName = nullptr;
      wc.lpszClassName = get_name();
      wc.hIconSm = (HICON)LoadImage(get_instance(), MAKEINTRESOURCE(IDI_ROXIICON), IMAGE_ICON, 16, 16, 0);
      RegisterClassEx(&wc);
    }

    Window::WindowClass::~WindowClass() {
      UnregisterClass(get_name(), get_instance());
    }

    HINSTANCE Window::WindowClass::get_instance() noexcept {
      return window_class.h_instance;
    }
 
    const char* Window::WindowClass::get_name() noexcept {
      return window_class.win_name;
    }

    b8 Window::init(u32 width, u32 height, const char* name) {
      RX_TRACE("initializing window and time");
      Time::init();
      extents.width = width;
      extents.height = height;

      RECT wr;
      wr.left = 100;
      wr.right = width + wr.left;
      wr.top = 100;
      wr.bottom = height + wr.top;
      if(AdjustWindowRect(&wr, WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU, FALSE) == 0) {
        return false;
      }

      RX_TRACE("creating window");
      h_wnd = CreateWindow(
          WindowClass::get_name()
        , name
        , WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU
        , CW_USEDEFAULT
        , CW_USEDEFAULT
        , wr.right - wr.left
        , wr.bottom - wr.top
        , nullptr
        , nullptr
        , WindowClass::get_instance()
        , this);

      if(h_wnd == nullptr) {
        return false;
      }

      RX_TRACE("showing window");
      ShowWindow(h_wnd, SW_SHOWDEFAULT);
      return true;
    }

    b8 Window::terminate() {
      RX_TRACE("destroying window and terminating time");
      DestroyWindow(h_wnd);
      Time::terminate();
      return true;
    }

    HINSTANCE Window::get_instance() noexcept {
      return WindowClass::get_instance();
    }

    HWND Window::get_window() noexcept {
      return h_wnd;
    }

    void Window::confine_cursor() noexcept {
      RECT rect; 
	    GetClientRect( h_wnd,&rect );
	    MapWindowPoints( h_wnd,nullptr,reinterpret_cast<POINT*>(&rect),2 );
	    ClipCursor( &rect );
    }
    
    void Window::free_cursor() noexcept {
      ClipCursor(nullptr);
    }

    void Window::show_cursor() noexcept {
      while(ShowCursor(TRUE) < 0);
    }

    void Window::hide_cursor() noexcept {
      while(ShowCursor(FALSE) >= 0);
    }

    void Window::enable_imgui_mouse() noexcept {

    }

    void Window::disable_imgui_mouse() noexcept {

    }

    Window::Extent Window::get_extents() const noexcept {
      return extents;
    }

    Window::Extent::operator VkExtent2D() {
      VkExtent2D extent{};
      extent.width = width;
      extent.height = height;
      return extent;
    }


    LRESULT CALLBACK Window::handle_msg_setup( HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam ) noexcept
    {
    	// use create parameter passed in from CreateWindow() to store window class pointer at WinAPI side
    	if( msg == WM_NCCREATE )
    	{
    		// extract ptr to window class from creation data
    		const CREATESTRUCTW* const pCreate = reinterpret_cast<CREATESTRUCTW*>(lParam);
    		Window* const pWnd = static_cast<Window*>(pCreate->lpCreateParams);
    		// set WinAPI-managed user data to store ptr to window instance
    		SetWindowLongPtr( hWnd,GWLP_USERDATA,reinterpret_cast<LONG_PTR>(pWnd) );
    		// set message proc to normal (non-setup) handler now that setup is finished
    		SetWindowLongPtr( hWnd,GWLP_WNDPROC,reinterpret_cast<LONG_PTR>(&Window::handle_msg_thunk) );
    		// forward message to window instance handler
    		return HANDLE_MSG( hWnd, msg,wParam,lParam, extents.width, extents.height);
    	}
    	// if we get a message before the WM_NCCREATE message, handle with default handler
    	return DefWindowProc( hWnd,msg,wParam,lParam );
    }

	  LRESULT CALLBACK Window::handle_msg_thunk(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
    {
    	// forward message to window instance handler
    	return HANDLE_MSG(hWnd, msg, wParam, lParam, extents.width, extents.height);
    }
 
  }		// -----  end of namespace win  ----- 

}		// -----  end of namespace roxi  ----- 
