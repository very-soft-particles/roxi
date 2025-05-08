// =====================================================================================
//
//       Filename:  vk_consts.h
//
//    Description:  
//
//        Version:  1.0
//        Created:  2023-11-27 3:02:00 PM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Roxi Graves (rg), nada
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#pragma once
// do OS defines BEFORE including vulkan (in vk_allocation_callbacks)
#include "vk_error.h"
#include "vk_allocation_callbacks.hpp"

#ifndef RX_USE_ALLOCATION_CALLBACKS
#define CALLBACKS() nullptr
#else
#define CALLBACKS() get_vk_callbacks()
#endif


#ifndef NVKDEBUG
#define VK_ASSERT(cond, statement) STMNT( VkResult r = (cond);   \
    if (r != VK_SUCCESS)                                  \
    { LOG(statement, Vulkan);                       \
      DUMP_LOGS();                                          \
      L_ASSERT_BREAK();                                     \
    })
#define VK_CHECK(cond, statement) STMNT( VkResult r = (cond); \
    if (r != VK_SUCCESS)                                \
    { LOG(statement, Vulkan);                            \
      return false;                                     \
    })
#define VK_RETURN_VAL(cond, statement, to_return) STMNT( VkResult r = (cond);\
    if(r != VK_SUCCESS)\
    { LOG(statement, Vulkan);\
      return to_return;\
    })
#else
#define VK_CHECK(cond, statement) (cond)
#define VK_ASSERT(cond, statement) (cond)
#define VK_RETURN(cond, statement, to_return) (cond)
#endif
