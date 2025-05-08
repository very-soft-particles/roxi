// =====================================================================================
//
//       Filename:  pch.h
//
//    Description:  
//
//        Version:  1.0
//        Created:  2024-04-28 9:16:33 AM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Roxi Graves (rg), nada
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#pragma once

#include "../../../lofi/core/include/l_definition.hpp"

#if OS_WINDOWS
#include <Windows.h>
#define VK_USE_PLATFORM_WIN32_KHR
#endif

#include <vulkan/vulkan.h>
#include "glm/glm.hpp"

