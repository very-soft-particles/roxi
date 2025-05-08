// =====================================================================================
//
//       Filename:  vk_library.h
//
//    Description:  
//
//        Version:  1.0
//        Created:  2023-11-27 3:01:33 PM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Roxi Graves (rg), nada
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#pragma once
#include "vk_consts.h"
#include "window.h"

namespace roxi {
  namespace vk {

#if OS_WINDOWS
    using LibraryType = HMODULE;
#define LoadFunction GetProcAddress
#elif defined __linux
    using LibraryType = void*;
#define LoadFunction dlsym
#endif

    struct WindowParams {
#if defined VK_USE_PLATFORM_WIN32_KHR

      HINSTANCE          HInstance;
      HWND               HWnd;

#elif defined VK_USE_PLATFORM_XLIB_KHR

      Display          * Dpy;
      Window             Window;

#elif defined VK_USE_PLATFORM_XCB_KHR

      xcb_connection_t * Connection;
      xcb_window_t       Window;

#endif
    };

    class Library {
    public:
      struct ExportedFunctionTable {
#define EXPORTED_VULKAN_FUNCTION( name ) PFN_##name name = nullptr;

#include "vk_function_list.inl"

#undef EXPORTED_VULKAN_FUNCTION
      };

      struct GlobalFunctionTable {
#define GLOBAL_LEVEL_VULKAN_FUNCTION( name ) PFN_##name name = nullptr;

#include "vk_function_list.inl"

#undef GLOBAL_LEVEL_VULKAN_FUNCTION
      };
    private:
      LibraryType _library = nullptr;
      ExportedFunctionTable _exported_function_table;
      GlobalFunctionTable _global_function_table;
    public:
      LibraryType get_library() const; 
      const ExportedFunctionTable& get_exported_function_table() const;
      const GlobalFunctionTable& get_global_function_table() const;
      b8 init();
      b8 terminate();

    private:
      b8 load_library();
      b8 load_exported_function();
      b8 load_global_level_functions();
    };

  }		// -----  end of namespace vk  ----- 
}		// -----  end of namespace roxi  ----- 
