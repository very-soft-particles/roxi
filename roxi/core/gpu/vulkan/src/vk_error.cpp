// =====================================================================================
//
//       Filename:  vk_error.cpp
//
//    Description:  
//
//        Version:  1.0
//        Created:  2024-01-25 3:43:38 PM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Roxi Graves (rg), nada
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#include "vk_error.h"
#include "rx_thread_pool.hpp"

namespace roxi {
  
  namespace vk {

    VkError::VkError(int line, const char* file, const char* message, VkResult result) : Error(line, file, message), result(result) {}

    const char* VkError::what() { 
      StringList list;
      str_list_push(
         &arena
        , &list
        , get_origin_string()
      );
      str_list_push(
         &arena
        , &list
        , get_message()
      );
      String vr = lofi::str_lit("VkResult = %d");
      str_list_pushf(&arena, &list, (char*)vr.str, result);
      return (char*)vr.str;
    }

    const char* VkError::get_type() const noexcept {
      return "roxi::vk::VkError";
    }

  }		// -----  end of namespace vk  ----- 

}		// -----  end of namespace roxi  ----- 
