// =====================================================================================
//
//       Filename:  vk_error.h
//
//    Description:  
//
//        Version:  1.0
//        Created:  2024-01-25 3:39:21 PM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Roxi Graves (rg), nada
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#pragma once
#include "error.h"

namespace roxi {
  namespace vk {
   
    class VkError : public Error {
    public:
      VkError(int line, const char* file, const char* message, VkResult result);
      virtual const char* what() override;
      virtual const char* get_type() const noexcept override;
    private:
      VkResult result;
    };

    // This is used in Instance::setup_debug_callbacks()
    static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
    	VkDebugUtilsMessageSeverityFlagBitsEXT Severity,
    	VkDebugUtilsMessageTypeFlagsEXT Type,
    	const VkDebugUtilsMessengerCallbackDataEXT* CallbackData,
    	void* UserData
    )
    {
    	LOG(CallbackData->pMessage, Vulkan);
    	return VK_FALSE;
    }
 
  }		// -----  end of namespace vk  ----- 
}		// -----  end of namespace roxi  ----- 
