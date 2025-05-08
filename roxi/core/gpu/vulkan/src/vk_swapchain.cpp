// =====================================================================================
//
//       Filename:  vk_swapchain.cpp
//
//    Description:  
//
//        Version:  1.0
//        Created:  2025-02-15 3:21:44 PM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Robyn Gray (rg), ...
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#include "vk_swapchain.h"
#include <vulkan/vulkan_core.h>

namespace roxi {
 
  namespace vk {

    b8 Surface::init(Instance* instance, void* wnd) {
#if OS_WINDOWS
      RX_TRACE("OS_WINDOWS detected in surface initialization");
      win::Window* p_wnd = (win::Window*)wnd;
      VkWin32SurfaceCreateInfoKHR create_info{};
      create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
      create_info.pNext = nullptr;
      create_info.flags = 0;
      create_info.hinstance = p_wnd->get_instance();
      create_info.hwnd = p_wnd->get_window();
      RX_TRACE("creating VkSurfaceKHR");
      if(instance->get_instance_function_table()
          .vkCreateWin32SurfaceKHR(instance->get_instance()
            , &create_info, CALLBACKS(), &_surface) != VK_SUCCESS) {
        LOG("failed to create win32 surface", Fatal);
        return false;
      }
#else
      RX_TRACE("OS detection returned null in surface initialization");
      return false
#endif
      return true;
    }

    b8 Surface::terminate(Instance* instance) {
      instance->get_instance_function_table()
        .vkDestroySurfaceKHR(instance->get_instance()
            , _surface
            , CALLBACKS());
      return true;
    }
    
    b8 Swapchain::init(Instance* instance, Device* device, void* wnd, const VkSurfaceKHR surface) {
      _presentation_surface = surface;

      VkSurfaceCapabilities2KHR surface_capabilities{};

      VkPhysicalDeviceSurfaceInfo2KHR physical_device_surface_info{};
      physical_device_surface_info.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SURFACE_INFO_2_KHR;
      physical_device_surface_info.pNext = nullptr;
      physical_device_surface_info.surface = _presentation_surface;


      surface_capabilities.sType = VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_2_KHR;
      instance->get_instance_function_table()
        .vkGetPhysicalDeviceSurfaceCapabilities2KHR(device->get_physical_device()
          , &physical_device_surface_info
          , &surface_capabilities);

      u32 format_count = 0;
      instance->get_instance_function_table()
        .vkGetPhysicalDeviceSurfaceFormats2KHR(device->get_physical_device()
            , &physical_device_surface_info
            , &format_count
            , nullptr);

      SizedStackArray<VkSurfaceFormat2KHR, 32> supported_formats;
      supported_formats.push(format_count);

      for(size_t i = 0; i < format_count; i++) {
        supported_formats[i].sType = VK_STRUCTURE_TYPE_SURFACE_FORMAT_2_KHR;
      }

      instance->get_instance_function_table()
        .vkGetPhysicalDeviceSurfaceFormats2KHR(device->get_physical_device()
            , &physical_device_surface_info
            , &format_count
            , supported_formats.get_buffer());

      VkSurfaceFormat2KHR chosen_format{};
      for(size_t i = 0; i < format_count; i++) {
        if(supported_formats[i].surfaceFormat.format == VK_FORMAT_B8G8R8A8_SRGB 
            && supported_formats[i].surfaceFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
          chosen_format = supported_formats[i];
          break;
        }
      }
      VkSwapchainCreateInfoKHR create_info{};

      create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
      create_info.pNext = nullptr;
      create_info.flags = 0;

      create_info.minImageCount = surface_capabilities.surfaceCapabilities.minImageCount + 1;
      if(RoxiNumFrames <= surface_capabilities.surfaceCapabilities.maxImageCount
          && RoxiNumFrames >= surface_capabilities.surfaceCapabilities.minImageCount) {
        create_info.minImageCount = RoxiNumFrames;
      }

      create_info.surface = _presentation_surface;
      create_info.clipped = VK_TRUE;
      create_info.presentMode = VK_PRESENT_MODE_FIFO_KHR;
      create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
      create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
      create_info.preTransform = surface_capabilities.surfaceCapabilities.currentTransform ?
        surface_capabilities.surfaceCapabilities.currentTransform : VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
      create_info.imageArrayLayers = 1;
#if OS_WINDOWS
        win::Window* p_wnd = (win::Window*)wnd;
        create_info.imageExtent = (VkExtent2D)p_wnd->get_extents();
#endif
        
        VkFormat format = chosen_format.surfaceFormat.format;
        create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        create_info.imageColorSpace = chosen_format.surfaceFormat.colorSpace;
        create_info.imageFormat = chosen_format.surfaceFormat.format;
        create_info.oldSwapchain = _swapchain;

        VK_CHECK(device->get_device_function_table()
          .vkCreateSwapchainKHR(device->get_device()
              , &create_info, CALLBACKS(), &_swapchain)
          , "failed to create swapchain khr");
        _current_extent = create_info.imageExtent;
        _swapchain_format = format;

        u32 image_count = 0;
        device->get_device_function_table()
          .vkGetSwapchainImagesKHR(device->get_device()
              , _swapchain, &image_count, nullptr);
        if(image_count == 0) {
          LOG("failed to get any images from swapchain", Fatal);
          return false;
        }
        device->get_device_function_table()
          .vkGetSwapchainImagesKHR(device->get_device()
            , _swapchain
            , &image_count
            , _swapchain_images.push(image_count));

        VkImageViewCreateInfo image_view_create_info{};
        image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        image_view_create_info.pNext = nullptr;
        image_view_create_info.flags = 0;
        image_view_create_info.format = chosen_format.surfaceFormat.format;
        image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        image_view_create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        image_view_create_info.subresourceRange.baseArrayLayer = 0;
        image_view_create_info.subresourceRange.baseMipLevel = 0;
        image_view_create_info.subresourceRange.layerCount = 1;
        image_view_create_info.subresourceRange.levelCount = 1;
        
        for(size_t i = 0; i < image_count; i++) {
          image_view_create_info.image = _swapchain_images[i];
          if(device->get_device_function_table()
            .vkCreateImageView(device->get_device()
                , &image_view_create_info, CALLBACKS(), _swapchain_image_views.push(1))
            != VK_SUCCESS) {
            LOG("failed to create swap chain image view", Fatal);
            return false;
          }
        }

        return true;
      

      return true;
    }

  }		// -----  end of namespace vk  ----- 

}		// -----  end of namespace roxi  ----- 
