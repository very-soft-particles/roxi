// =====================================================================================
//
//       Filename:  vk_context.h
//
//    Description:  
//
//        Version:  1.0
//        Created:  2024-04-29 2:28:08 PM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Roxi Graves (rg), nada
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#pragma once
#include "vk_device.h"
#include "vk_swapchain.h"

namespace roxi {
  
  namespace vk {

    // composition of library, instance and device builders
    class ContextBuilder;

    class Context {
    private:
      friend class ContextBuilder;
      b8 create_library();
      b8 create_instance();
      b8 create_device();
      b8 create_surface(void* wnd);
      b8 create_swapchain(void* wnd);
      Library _library;
      Instance _instance;
      Surface _surface;
      Swapchain _swapchain;
      Device _device;
    public:
      Context() {};
      ~Context() {};
      b8 init(void* wnd);
      b8 terminate();
      const Library& get_library() const {
        return _library;
      }
      const Instance& get_instance() const {
        return _instance;
      }
      const Device& get_device() const {
        return _device;
      }
      const Swapchain& get_swapchain() const {
        return _swapchain;
      }
      const Surface& get_presentation_surface() const {
        return _surface;
      }
      const VkQueue get_queue(const u32 queue_handle) const {
        return _device.get_queue(queue_handle);
      }
      const Library& get_library() {
        return _library;
      }
      const Instance& get_instance() {
        return _instance;
      }
      const Device& get_device() {
        return _device;
      }
      const Swapchain& get_swapchain() {
        return _swapchain;
      }
      const Surface& get_presentation_surface() {
        return _surface;
      }
      const VkQueue get_queue(const u32 queue_handle) {
        return _device.get_queue(queue_handle);
      }
    };

    class ContextBuilder {
    private:
      InstanceBuilder _instance_builder;
      DeviceBuilder _device_builder;
      void* _wnd = nullptr;
    public:

      b8 init();

      ContextBuilder& set_name(const char* name);
      ContextBuilder& set_window(void* window);

      ContextBuilder& add_instance_layer(const char* layer_name);
      ContextBuilder& add_device_layer(const char* layer_name);

      ContextBuilder& add_instance_extension(const char* extension_name);
      ContextBuilder& add_device_extension(const char* extension_name);

      const u32 add_queue_type(const QueueType queue_type);

      ContextBuilder& add_device_p_next(void* next);
      ContextBuilder& add_instance_p_next(void* next);

      b8 build(Context* context);

      b8 terminate();

    };

  }		// -----  end of namespace vk  ----- 

}		// -----  end of namespace roxi  ----- 
