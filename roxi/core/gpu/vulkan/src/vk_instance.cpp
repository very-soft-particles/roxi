// =====================================================================================
//
//       Filename:  vk_instance.cpp
//
//    Description:  
//
//        Version:  1.0
//        Created:  2024-04-29 5:09:06 PM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Roxi Graves (rg), nada
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#include "vk_instance.h"
#include "rx_allocator.hpp"
#include "rx_log.hpp"
#include "vk_consts.h"
#include <vulkan/vulkan_core.h>


namespace roxi {
  
  namespace vk {
    b8 Instance::terminate() {
      if(_instance != VK_NULL_HANDLE) {
        _instance_function_table.vkDestroyInstance(_instance, CALLBACKS());
        _instance = VK_NULL_HANDLE;
      }
      return true;
    }

    void InstanceBuilder::set_extensions(StringList* new_extension_names) {
      for(StringNode* node = new_extension_names->first;
          node != nullptr;
          node = node->next) {
        str_list_push(&_scratch, &_enabled_extensions, node->string);
      }
    }

    void InstanceBuilder::set_library(Library* library) {
      _library = library;
    }

    b8 InstanceBuilder::init() {
      // allocate string buffer
      _scratch.move_ptr(ALLOCATE(KB(16)));
      _p_next_chain.move_ptr(ALLOCATE(sizeof(void*) * 32));
      // in case no p nexts get added
      _p_next_chain[0] = nullptr;
      return true;
    }

    b8 InstanceBuilder::terminate() {
      FREE((void*)_scratch.get_buffer());
      FREE((void*)_p_next_chain.get_buffer());
      return true;
    }

    void InstanceBuilder::set_layers(StringList* new_layer_names) {
      for(StringNode* node = new_layer_names->first;
          node != nullptr;
          node = node->next) {
        str_list_push(&_scratch, &_enabled_layers, node->string);
      }
    }

    void InstanceBuilder::add_extension(const char* new_extension) {
      String string = lofi::str_cstring(new_extension);
      str_list_push(&_scratch, &_enabled_extensions, string);
    }

    void InstanceBuilder::add_layer(const char* new_layer) {
      String string = lofi::str_cstring(new_layer);
      str_list_push(&_scratch, &_enabled_layers, string);
    }

    b8 InstanceBuilder::build(Instance* instance) {

      add_extension("VK_EXT_debug_utils");

      VkApplicationInfo app_info{};
      app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
      app_info.pNext = nullptr;
      app_info.pEngineName = "roxi";
      app_info.pApplicationName = _app_name ? _app_name : "default app name";
      app_info.applicationVersion = _vulkan_version;
      app_info.apiVersion = VK_API_VERSION_1_3;
      app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);

      StringList exts_to_use = get_extensions_to_use();
      StringList layers_to_use = get_layers_to_use();

      VkInstanceCreateInfo create_info{};
      Array<char*> ext_array(_scratch.push(sizeof(char*) * exts_to_use.node_count));
      Array<char*> layer_array(_scratch.push(sizeof(char*) * layers_to_use.node_count));
      create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
      create_info.pNext = _p_next_chain[0];
      create_info.flags = 0;
      create_info.enabledExtensionCount = static_cast<u32>(exts_to_use.node_count);
      for(auto node = exts_to_use.first; node != nullptr; node = node->next) {
        //auto ptr = ext_array.push(1);
        *(ext_array.push(1)) = (char*)node->string.str;
      }
      create_info.ppEnabledExtensionNames = (char**)(ext_array.get_buffer());
      create_info.enabledLayerCount = static_cast<u32>(layers_to_use.node_count);
      for(auto node = layers_to_use.first; node != nullptr; node = node->next) {
        //auto ptr = layer_array.push(1);
        *(layer_array.push(1)) = (char*)node->string.str;
      }
      create_info.ppEnabledLayerNames = (char**)layer_array.get_buffer();
 
      create_info.pApplicationInfo = &app_info;

      VK_CHECK(
        _library->get_global_function_table()
          .vkCreateInstance
            ( &create_info
            , CALLBACKS()
            , &instance->_instance)
        , "unable to create VkInstance");
      if(instance->_instance == VK_NULL_HANDLE) {
        RX_ERROR("vkCreateInstance returned NULL");
        return false;
      }

#define INSTANCE_LEVEL_VULKAN_FUNCTION( name )                  \
    instance->_instance_function_table.name = (PFN_##name)vkGetInstanceProcAddr( instance->_instance, #name );\
    if( instance->_instance_function_table.name == nullptr ) {                                               \
      LOG("Could not load instance level Vulkan function named: "#name, Warn);\
      return false;                                             \
    }

#include "vk_function_list.inl"


#define INSTANCE_LEVEL_VULKAN_FUNCTION_FROM_EXTENSION( name, extension )   \
    for(const StringNode* node = exts_to_use.first;                        \
        node != nullptr;                                                   \
        node = node->next) {                                               \
      if(lofi::str_compare(node->string, lofi::str_lit(extension))) {      \
        instance->_instance_function_table.name =                          \
        (PFN_##name)vkGetInstanceProcAddr( instance->_instance, #name );   \
      }                                                                    \
                                                                           \
    }                                                                      \
    if( instance->_instance_function_table.name == nullptr ) {                                               \
      LOG("Could not load instance level Vulkan extension function named: "#name" with extension: " extension, Warn);\
      return false;                                                       \
    }

#include "vk_function_list.inl"

      VkDebugUtilsMessengerCreateInfoEXT debug_messenger_create_info{VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT};

      debug_messenger_create_info.pNext = nullptr;
      debug_messenger_create_info.messageType 
        = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
      debug_messenger_create_info.messageSeverity 
        = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
      debug_messenger_create_info.pfnUserCallback = debug_callback;
      debug_messenger_create_info.pUserData = nullptr;

      VK_CHECK(
        instance->_instance_function_table.vkCreateDebugUtilsMessengerEXT
          ( instance->_instance
          , &debug_messenger_create_info
          , CALLBACKS()
          , &instance->_debug_messenger
          )
        , "failed to create vulkan debug messenger"
        );

      return true;
    }

    StringList InstanceBuilder::get_available_extensions() {

      StringList available_extensions;

      u32 count = 0;

      auto g_function_table = _library->get_global_function_table();
      VK_ASSERT(g_function_table.vkEnumerateInstanceExtensionProperties(
            nullptr, &count, nullptr), "could not enumerate any instance extension properties");
      Array<VkExtensionProperties> ext_props(_scratch.push(sizeof(VkExtensionProperties) * count));
      VK_ASSERT(g_function_table.vkEnumerateInstanceExtensionProperties(
            nullptr, &count, ext_props.push(count)), "error writing instance extension properties");

      for(size_t i = 0; i < ext_props.get_size(); i++) {
        String str = lofi::str_cstring(ext_props[i].extensionName);
        str_list_push(&_scratch, &available_extensions, str);
      }

      return available_extensions;
    }
 
    StringList InstanceBuilder::get_available_layers() {
      StringList available_layers;

      u32 count = 0;

      auto g_function_table = _library->get_global_function_table();
      VK_ASSERT(g_function_table.vkEnumerateInstanceLayerProperties(
            &count, nullptr), "could not enumerate any Instance layer properties");
      Array<VkLayerProperties> layer_props(_scratch.push(sizeof(VkLayerProperties) * count));
      VK_ASSERT(g_function_table.vkEnumerateInstanceLayerProperties(
            &count, layer_props.push(count)), "error writing instance layer properties");

      for(size_t i = 0; i < layer_props.get_size(); i++) {
        String str = lofi::str_cstring(layer_props[i].layerName);
        str_list_push(&_scratch, &available_layers, str);
      }

      return available_layers;
    }

    StringList InstanceBuilder::get_extensions_to_use() {

      StringList avail_exts = get_available_extensions();

      StringList exts_to_use;

      for(const StringNode* nodei = _enabled_extensions.first;
          nodei != nullptr;
          nodei = nodei->next) {
        String string = nodei->string;
        for(const StringNode* nodej = avail_exts.first;
            nodej != nullptr;
            nodej = nodej->next) {
          if(str_compare(string, nodej->string)) {
            str_list_push(&_scratch, &exts_to_use, string);
            break;
          }
        }
      }
      return exts_to_use;
    }

    StringList InstanceBuilder::get_layers_to_use() {

      StringList avail_layers = get_available_layers();

      StringList layers_to_use;

      for(const StringNode* nodei = _enabled_layers.first;
          nodei != nullptr;
          nodei = nodei->next) {
        String string = nodei->string;
        for(const StringNode* nodej = avail_layers.first;
            nodej != nullptr;
            nodej = nodej->next) {
          if(str_compare(string, nodej->string)) {
            str_list_push(&_scratch, &layers_to_use, string);
          }
        }
      }
      return layers_to_use;
    }

    void InstanceBuilder::set_version(u32 new_version) {
      _vulkan_version = new_version;
    }
    void InstanceBuilder::set_name(const char* new_name) {
      _app_name = new_name;
    }

  }		// -----  end of namespace vk  ----- 

}		// -----  end of namespace roxi  ----- 
