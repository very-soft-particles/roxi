include("${CMAKE_CURRENT_LIST_DIR}/../cmake/config/pch_config.cmake")
project(roxi)

set(glm_DIR "C:/vcpkg/vcpkg/packages/glm_x64-windows/share/glm")
find_package(Vulkan REQUIRED)
find_package(glm CONFIG REQUIRED)


message("Vulkan Library: " ${Vulkan_LIBRARY})
message("Vulkan include: " ${Vulkan_INCLUDE_DIR})
message("Roxi lib name: " ${PROJECT_NAME})

set(PRIVATE_PRECOMPILED_HEADERS 
    [["vulkan/vulkan.h"]]
    [["glm/glm.hpp"]]
    <Windows.h>
  )

set(PUBLIC_PRECOMPILED_HEADERS 
  )

set(INTERFACE_PRECOMPILED_HEADERS 
  #  [["lofi.hpp"]]
  )

# rlsl not ready
#set(ROXI_RLSL_PATH "${CMAKE_CURRENT_LIST_DIR}/core/rlsl/")

set(ROXI_COMMON_PATH "${CMAKE_CURRENT_LIST_DIR}/core/common/")
set(ROXI_MATH_PATH "${CMAKE_CURRENT_LIST_DIR}/core/math/")
set(ROXI_GPU_PATH "${CMAKE_CURRENT_LIST_DIR}/core/gpu/")
set(ROXI_RENDERER_PATH "${CMAKE_CURRENT_LIST_DIR}/core/renderer/")
set(ROXI_SYSTEMS_PATH "${CMAKE_CURRENT_LIST_DIR}/core/systems/")
set(ROXI_VULKAN_PATH "${ROXI_GPU_PATH}vulkan/")
set(ROXI_ECS_PATH "${CMAKE_CURRENT_LIST_DIR}/core/ecs/")
set(ROXI_WINDOWS_PATH "${CMAKE_CURRENT_LIST_DIR}/core/windows/")
set(ROXI_FRAME_PATH "${CMAKE_CURRENT_LIST_DIR}/core/frame/")
set(ROXI_RESOURCE_PATH "${CMAKE_CURRENT_LIST_DIR}/core/resource/")
set(ROXI_SCRIPT_PATH "${CMAKE_CURRENT_LIST_DIR}/core/scripting/")

set(ROXI_INCLUDE_PATHS
  "${CMAKE_CURRENT_LIST_DIR}/include/"
  )

set(ROXI_SYSTEMS_LIST
  "${ROXI_SYSTEMS_PATH}"
  "${ROXI_SYSTEMS_PATH}graphics/"
  "${ROXI_SYSTEMS_PATH}physics/"
  )

set(ROXI_INTERNAL_INCLUDE_PATHS
  "${CMAKE_CURRENT_LIST_DIR}/../lofi/include/"
  "${ROXI_COMMON_PATH}include/"
  "${ROXI_MATH_PATH}include/"
  "${ROXI_VULKAN_PATH}include/"
  "${ROXI_WINDOWS_PATH}include/"
  "${ROXI_ECS_PATH}include/"
  "${ROXI_RENDERER_PATH}include/"
  "${ROXI_GPU_PATH}include/"
  "${ROXI_GPU_PATH}data/"
  "${ROXI_RESOURCE_PATH}"
  "${ROXI_SYSTEMS_LIST}"
  "${ROXI_SCRIPT_PATH}"
  "${ROXI_FRAME_PATH}"
  )

set(ROXI_SRC_PATHS
  "${CMAKE_CURRENT_LIST_DIR}/src"
  "${ROXI_VULKAN_PATH}src/"
  )

set(ROXI_SRC_FILES  

  "${ROXI_COMMON_PATH}include/pch.h"

  "${ROXI_FRAME_PATH}rx_frame_manager.hpp"
  "${ROXI_COMMON_PATH}include/rx_vocab.h"
  "${ROXI_COMMON_PATH}include/error.h"
  "${ROXI_COMMON_PATH}include/rx_allocator.hpp"
  "${ROXI_COMMON_PATH}include/rx_time.h"
  "${ROXI_COMMON_PATH}include/rx_arena.hpp"
  "${ROXI_COMMON_PATH}include/rx_container.hpp"
  "${ROXI_COMMON_PATH}include/rx_string.hpp"
  "${ROXI_COMMON_PATH}include/rx_thread_pool.hpp"
  "${ROXI_COMMON_PATH}include/rx_tuple.hpp"
  "${ROXI_COMMON_PATH}include/rx_variant.hpp"
  "${ROXI_COMMON_PATH}include/rx_log.hpp"
  "${ROXI_COMMON_PATH}include/rx_file.hpp"
  "${ROXI_COMMON_PATH}include/rx_input.h"
 
  "${ROXI_COMMON_PATH}src/error.cpp"
  "${ROXI_COMMON_PATH}src/rx_time.cpp"
  "${ROXI_COMMON_PATH}src/rx_log.cpp"
  "${ROXI_COMMON_PATH}src/rx_allocator.cpp"
  "${ROXI_COMMON_PATH}src/rx_arena.cpp"
  "${ROXI_COMMON_PATH}src/rx_thread_pool.cpp"

  "${ROXI_WINDOWS_PATH}include/message_handler.h"
  "${ROXI_WINDOWS_PATH}include/window.h"

  "${ROXI_WINDOWS_PATH}src/message_handler.cpp"
  "${ROXI_WINDOWS_PATH}src/window.cpp"
  "${ROXI_RESOURCE_PATH}resource.h"
  "${ROXI_RESOURCE_PATH}roxi.rc"

  "${ROXI_RESOURCE_PATH}ecs_resources.hpp"
  "${ROXI_RESOURCE_PATH}audio_resources.hpp"
  "${ROXI_RESOURCE_PATH}shader_resources.hpp"
  "${ROXI_RESOURCE_PATH}mesh_resources.hpp"
  "${ROXI_RESOURCE_PATH}rx_resource_manager.hpp"

  "${ROXI_ECS_PATH}/include/rx_ecs.hpp"
  "${ROXI_ECS_PATH}/src/rx_ecs.cpp"

  "${ROXI_MATH_PATH}include/rx_math.hpp"
  "${ROXI_MATH_PATH}src/rx_math.cpp"
  
  "${ROXI_VULKAN_PATH}include/vk_function_list.inl"
  "${ROXI_VULKAN_PATH}include/vk_consts.h"
  "${ROXI_VULKAN_PATH}include/vk_allocation_callbacks.hpp"
  "${ROXI_VULKAN_PATH}include/vk_allocator.hpp"
  "${ROXI_VULKAN_PATH}include/vk_library.h"
  "${ROXI_VULKAN_PATH}include/vk_instance.h"
  "${ROXI_VULKAN_PATH}include/vk_swapchain.h"
  "${ROXI_VULKAN_PATH}include/vk_device.h"
  "${ROXI_VULKAN_PATH}include/vk_context.h"
  "${ROXI_VULKAN_PATH}include/vk_command.hpp"
  "${ROXI_VULKAN_PATH}include/vk_renderpass.hpp"
  "${ROXI_VULKAN_PATH}include/vk_pipeline.hpp"
  "${ROXI_VULKAN_PATH}include/vk_resource.hpp"
  "${ROXI_VULKAN_PATH}include/vk_descriptors.hpp"
  "${ROXI_VULKAN_PATH}include/vk_barrier.hpp"
  "${ROXI_VULKAN_PATH}include/vk_buffer.hpp"
  "${ROXI_VULKAN_PATH}include/vk_image.hpp"
  "${ROXI_VULKAN_PATH}include/vk_error.h"
  "${ROXI_VULKAN_PATH}include/vk_sync.hpp"
  "${ROXI_VULKAN_PATH}include/vk_batch.hpp"
  

  "${ROXI_VULKAN_PATH}src/vk_consts.cpp"
  "${ROXI_VULKAN_PATH}src/vk_error.cpp"
  "${ROXI_VULKAN_PATH}src/vk_allocation_callbacks.cpp"
  "${ROXI_VULKAN_PATH}src/vk_allocator.cpp"
  "${ROXI_VULKAN_PATH}src/vk_library.cpp"
  "${ROXI_VULKAN_PATH}src/vk_instance.cpp"
  "${ROXI_VULKAN_PATH}src/vk_device.cpp"
  "${ROXI_VULKAN_PATH}src/vk_context.cpp"
  "${ROXI_VULKAN_PATH}src/vk_command.cpp"
  "${ROXI_VULKAN_PATH}src/vk_renderpass.cpp"
  "${ROXI_VULKAN_PATH}src/vk_resource.cpp"
  "${ROXI_VULKAN_PATH}src/vk_swapchain.cpp"
  "${ROXI_VULKAN_PATH}src/vk_barrier.cpp"
  "${ROXI_VULKAN_PATH}src/vk_buffer.cpp"
  "${ROXI_VULKAN_PATH}src/vk_image.cpp"
  "${ROXI_VULKAN_PATH}src/vk_descriptors.cpp"
  "${ROXI_VULKAN_PATH}src/vk_pipeline.cpp"
  #"${ROXI_RENDERER_PATH}/include/rx_rendergraph.hpp"
  "${ROXI_GPU_PATH}/include/rx_gpu_device.hpp"
  "${ROXI_GPU_PATH}data/data.hpp"
  "${ROXI_RENDERER_PATH}/include/rx_renderer.hpp"

  #"${ROXI_RENDERER_PATH}/src/rx_rendergraph.cpp"
  "${ROXI_GPU_PATH}/src/rx_gpu_device.cpp"
  "${ROXI_RENDERER_PATH}/src/rx_renderer.cpp"

  # sus scripting shit
  #"${ROXI_SCRIPT_PATH}rx_lua.hpp"
  #"${ROXI_SCRIPT_PATH}/src/rx_dynamic_ecs.cpp"
  #"${ROXI_SCRIPT_PATH}/src/rx_system_graph.cpp"
  "${ROXI_SYSTEMS_PATH}graphics/rx_graphics.hpp"
  "${ROXI_SYSTEMS_PATH}physics/rx_physics.hpp"
 
  "${ROXI_SYSTEMS_PATH}rx_system.hpp"


  "${CMAKE_CURRENT_LIST_DIR}/include/Application.h"
  "${CMAKE_CURRENT_LIST_DIR}/src/Application.cpp"
  )

add_library(${PROJECT_NAME}
  SHARED
    ${ROXI_SRC_FILES}
  )

target_include_directories(${PROJECT_NAME}
  PUBLIC
  #${LUA_INCLUDE}
   "${ROXI_INCLUDE_PATHS}"
   "${ROXI_INTERNAL_INCLUDE_PATHS}"
  PRIVATE
   "${LOFI_INCLUDE_PATH}"
   "${ROXI_SRC_PATHS}"
  )

message("Roxi lib name: " ${PROJECT_NAME})

target_link_libraries(
  ${PROJECT_NAME}
  PRIVATE
    Vulkan::Vulkan
    glm::glm
    LOFI
  #  Lua
  )

update_pch_config()

