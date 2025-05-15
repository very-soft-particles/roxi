// =====================================================================================
//
//       Filename:  vk_swapchain.h
//
//    Description:  
//
//        Version:  1.0
//        Created:  2024-05-09 8:42:38 PM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Roxi Graves (rg), nada
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#pragma once
#include "vk_consts.h"
#include <vulkan/vulkan_core.h>
#if OS_WINDOWS
#include "window.h"
#endif
#include "rx_frame_manager.hpp"
#include "vk_device.h"


namespace roxi {

  namespace vk {

    class Surface {
    private:
      VkSurfaceKHR _surface;
    public:
      b8 init(const Instance* instance, void* wnd);

      b8 terminate(Instance* instance);

      const VkSurfaceKHR& get_surface() {
        return _surface;
      }
    };

    class Swapchain {
    private:
      friend class SwapchainBuilder;
      VkSwapchainKHR _swapchain = VK_NULL_HANDLE;
      VkSurfaceKHR _presentation_surface = VK_NULL_HANDLE;
      VkExtent2D _current_extent{};
      VkFormat _swapchain_format{};
      SizedStackArray<VkImage, RoxiNumFrames> _swapchain_images;
      SizedStackArray<VkImageView, RoxiNumFrames> _swapchain_image_views;

    public:

      b8 init(const Instance* instance, const Device* device, void* wnd, const VkSurfaceKHR surface);

      // returns false if no image is available, use signal semaphore to synchronize writing to image
      b8 acquire_next_image_index(const Device* device, u32* index_out, VkSemaphore signal_semaphore) const;

      const VkSwapchainKHR& get_swapchain() const {
        return _swapchain;
      }

      VkSurfaceKHR get_surface() const {
        return _presentation_surface;
      }

      VkExtent2D get_current_extent() const {
        return _current_extent;
      }

      const VkImageView get_attachment(const u32 idx) const {
        return _swapchain_image_views[idx];
      }

      const VkImage get_image(const u32 idx) const {
        return _swapchain_images[idx];
      }

      const u32 get_attachment_count() const {
        return _swapchain_image_views.get_size();
      }

      const VkFormat get_swapchain_format() const {
        return _swapchain_format;
      }

      b8 terminate(Device* device) {
        if(_swapchain == VK_NULL_HANDLE) {
          return false;
        }
        const u32 image_count = _swapchain_images.get_size();
        for(u32 i = 0; i < image_count; i++) {
          device->get_device_function_table()
            .vkDestroyImage(device->get_device()
                , _swapchain_images[i]
                , CALLBACKS());
        }
        const u32 image_view_count = _swapchain_image_views.get_size();
        for(u32 i = 0; i < image_view_count; i++) {
          device->get_device_function_table()
            .vkDestroyImageView(device->get_device()
                , _swapchain_image_views[i]
                , CALLBACKS());
        }
        device->get_device_function_table()
        .vkDestroySwapchainKHR(device->get_device()
            , _swapchain
            , CALLBACKS());

        return true;
      }
    private:

    };

  }		// -----  end of namespace vk  ----- 

}		// -----  end of namespace roxi  ----- 
