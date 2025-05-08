// =====================================================================================
//
//       Filename:  vk_instance.h
//
//    Description:  
//
//        Version:  1.0
//        Created:  2023-11-27 3:42:46 PM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Roxi Graves (rg), nada
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#pragma once
#include "vk_library.h"
#include <vulkan/vulkan_core.h>

namespace roxi {
  
  namespace vk {

    class InstanceBuilder;

    class Instance {
      friend class InstanceBuilder;
    public:
      struct FunctionTable {

#define INSTANCE_LEVEL_VULKAN_FUNCTION( name ) PFN_##name name;

#define INSTANCE_LEVEL_VULKAN_FUNCTION_FROM_EXTENSION( name, extension ) PFN_##name name = nullptr;

#include "vk_function_list.inl"

#undef INSTANCE_LEVEL_VULKAN_FUNCTION
#undef INSTANCE_LEVEL_VULKAN_FUNCTION_FROM_EXTENSION

      };
      
    private:
      VkInstance _instance = VK_NULL_HANDLE;
      FunctionTable _instance_function_table;
      VkDebugUtilsMessengerEXT _debug_messenger = VK_NULL_HANDLE;
    public:
      b8 terminate();

      const VkInstance& get_instance() const { return _instance; };
      const FunctionTable& get_instance_function_table() const { return _instance_function_table; };
    private:
    };
 
    class InstanceBuilder {
    public:

    private:
      Arena _scratch;
      StringList get_available_extensions();
      StringList get_available_layers();
   
      StringList _enabled_extensions;
      StringList _enabled_layers;

      StringList get_extensions_to_use();
      StringList get_layers_to_use();

      Array<void*> _p_next_chain;

      const char* _app_name = nullptr;
      u32 _vulkan_version;
      Library* _library;
      void setup_debug_callbacks();
 
    public:
      b8 init();
      void set_library(Library* library);
      
      void set_extensions(StringList* new_extension_names);
      void set_layers(StringList* new_layer_names);

      void set_version(u32 new_version);
      void set_name(const char* new_name);

      void add_extension(const char* new_extension);
      void add_layer(const char* new_layer);

      void add_p_next(void* next) {
        const u32 p_next_size = _p_next_chain.get_size();
        *(_p_next_chain.push(1)) = (void*)next;
        if(p_next_size) {
          u8* p_next = (u8*)(_p_next_chain[p_next_size - 1]) + (sizeof(VkStructureType));
          *(void**)p_next = next;
        }
      }

      b8 build(Instance* instance);
      b8 terminate();
    };

  }		// -----  end of namespace vk  ----- 

}		// -----  end of namespace roxi  ----- 
