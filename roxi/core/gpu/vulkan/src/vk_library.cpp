// =====================================================================================
//
//       Filename:  vk_library.cpp
//
//    Description:  
//
//        Version:  1.0
//        Created:  2024-04-29 3:58:34 PM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Roxi Graves (rg), nada
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#include "../include/vk_library.h"

namespace roxi {
  
  namespace vk {

    b8 Library::init() {
      return load_library() && load_exported_function() && load_global_level_functions();
    }

    b8 Library::terminate() {
      if(_library != nullptr) {
#if defined _WIN32
    FreeLibrary( _library );
#elif defined __linux
    dlclose( _library );
#endif
      }
      _library = nullptr;
      return true;
    }

    LibraryType Library::get_library() const { return _library; }

    const typename Library::ExportedFunctionTable& Library::get_exported_function_table() const { return _exported_function_table; }

    const typename Library::GlobalFunctionTable& Library::get_global_function_table() const { return _global_function_table; }


    b8 Library::load_library() { 
#if defined _WIN32
    _library = LoadLibrary( "vulkan-1.dll" );
#elif defined __linux
    _library->_library = dlopen( "libvulkan.so.1", RTLD_NOW );
#endif

      if(_library == nullptr) {
        LOG("Could not connect with a Vulkan Runtime library", Fatal);
        return false;
      }
      return true;
    }

    b8 Library::load_exported_function() {
      SizedStackArena<1024> scratch;
#define EXPORTED_VULKAN_FUNCTION( name )                              \
    _exported_function_table.name = (PFN_##name)LoadFunction( _library, #name );         \
    if( name == nullptr ) {                                           \
      LOG("Could not load exported Vulkan function named: "#name"\n", Fatal);                      \
      return false;                                                   \
    }

#include "vk_function_list.inl"
#undef EXPORTED_VULKAN_FUNCTION

      return true;
    }

    b8 Library::load_global_level_functions() {
      SizedStackArena<1024> scratch;
#define GLOBAL_LEVEL_VULKAN_FUNCTION( name )                              \
    _global_function_table.name = (PFN_##name)vkGetInstanceProcAddr( nullptr, #name );\
    if( name == nullptr ) {                                               \
      LOG("Could not load global level Vulkan function named: "#name"\n", Fatal);                      \
      return false;                                                       \
    }

#include "vk_function_list.inl"
#undef GLOBAL_LEVEL_VULKAN_FUNCTION

        return true;
    }

  }		// -----  end of namespace vk  ----- 

}		// -----  end of namespace roxi  ----- 
