cmake_minimum_required(VERSION 3.23)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_SIZEOF_VOID_P 8)

include("${CMAKE_CURRENT_LIST_DIR}/cmake/config/config_macro.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/cmake/config/shader_config_macro.cmake")

project(ROXI_CONFIG)

find_package(Vulkan REQUIRED)

set(CONFIG_INCLUDE_PATH "${CMAKE_CURRENT_LIST_DIR}/include/")

generate_config(graphics_config)
generate_config(audio_config)
generate_config(ecs_config)
generate_shader_config(shader_config)


