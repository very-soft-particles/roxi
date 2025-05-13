// =====================================================================================
//
//       Filename:  vk_context.cpp
//
//    Description:  
//
//        Version:  1.0
//        Created:  2024-05-02 12:42:28 PM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Roxi Graves (rg), nada
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#include "vk_context.h"
#include <error.h>


namespace roxi {
  
  namespace vk {

    b8 Context::terminate() {
      RX_TRACE("terminating swapchain");
      _swapchain.terminate(&_device);
      RX_TRACE("terminating device");
      _device.terminate();
      RX_TRACE("terminating surface");
      _surface.terminate(&_instance);
      RX_TRACE("terminating instance");
      _instance.terminate();
      RX_TRACE("terminating library");
      _library.terminate();
      return true;
    }

    b8 ContextBuilder::init() {
      RX_CHECK(_instance_builder.init()
          , "roxi::vk::ContextBuilder failed to initialize roxi::vk::InstanceBuilder");
      RX_CHECK(_device_builder.init()
          , "roxi::vk::ContextBuilder failed to initialize roxi::vk::DeviceBuilder");
      return true;
    }

    ContextBuilder& ContextBuilder::set_name(const char* name) {
      _instance_builder.set_name(name);
      return *this;
    }

    ContextBuilder& ContextBuilder::set_window(void* window) {
      _wnd = window;
      return *this;
    }

    ContextBuilder& ContextBuilder::add_instance_extension(const char* extension_name) {
      _instance_builder.add_extension(extension_name);
      return *this;
    }
      
    ContextBuilder& ContextBuilder::add_device_extension(const char* extension_name) {
      _device_builder.add_extension(extension_name);
      return *this;
    }

    ContextBuilder& ContextBuilder::add_instance_layer(const char* layer_name) {
      _instance_builder.add_layer(layer_name);
      return *this;
    }

    ContextBuilder& ContextBuilder::add_device_layer(const char* layer_name) {
      _device_builder.add_layer(layer_name);
      return *this;
    }

    ContextBuilder& ContextBuilder::add_device_p_next(void* next) {
      _device_builder.add_p_next(next);
      return *this;
    }

    ContextBuilder& ContextBuilder::add_instance_p_next(void* next) {
      _instance_builder.add_p_next(next);
      return *this;
    }

    const u32 ContextBuilder::add_queue_type(const QueueType queue_type) {
      return _device_builder.register_queue(queue_type);
    }

    b8 ContextBuilder::build(Context* context) {

      RX_TRACE("initializing roxi::vk::Library");
      RX_CHECK(context->_library.init()
          , "roxi::vk::ContextBuilder failed to init roxi::vk::Library");
      _instance_builder.set_library(&context->_library);

      RX_TRACE("building roxi::vk::Instance");
      RX_CHECK(_instance_builder.build(&context->_instance)
          , "roxi::vk::ContextBuilder failed to build roxi::vk::Instance");
      _device_builder.set_instance(&context->_instance);

      RX_TRACE("initializing roxi::vk::Surface");
      RX_CHECK(context->_surface.init(&context->_instance, _wnd), "roxi::vk::ContextBuilder failed to build roxi::vk::Surface");

      _device_builder.set_surface(context->_surface.get_surface());
      _device_builder.add_extension("VK_EXT_descriptor_buffer");

      RX_TRACE("building roxi::vk::Device");
      RX_CHECK(_device_builder.build(&context->_device)
          , "roxi::vk::ContextBuilder failed to build roxi::vk::Device");

      RX_TRACE("initializing roxi::vk::Swapchain");
      RX_CHECK(context->_swapchain.init(&context->_instance, &context->_device, _wnd, context->_surface.get_surface()), "roxi::vk::ContextBuilder failed to build roxi::vk::Swapchain");

      return true;
    }

    b8 ContextBuilder::terminate() {
      RX_CHECK(_device_builder.terminate()
          , "roxi::vk::ContextBuilder failed to terminate roxi::vk::DeviceBuilder");

      RX_CHECK(_instance_builder.terminate()
          , "roxi::vk::ContextBuilder failed to terminate roxi::vk::InstanceBuilder");
      return true;
    }
  }		// -----  end of namespace vk  ----- 
}		// -----  end of namespace roxi  ----- 
