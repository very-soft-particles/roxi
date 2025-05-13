cmake_minimum_required(VERSION 3.31)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# must set size of VOID_P in order for ninja to work, regular build sets automatically
set(CMAKE_SIZEOF_VOID_P 8)

project(roxi_0)
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/config/audio_config.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/config/graphics_config.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/config/ecs_config.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/config/shader_config.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/config/compile_shaders.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/config/compile_prebuilds.cmake")


set(LOFI_NO_TESTS)

set(PREBUILD_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/prebuild/")
set(RESOURCE_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/roxi/core/resource/")
set(SHADER_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/roxi/core/renderer/shaders/")

set(SHADER_INCLUDE_DIRECTORIES
  "${SHADER_DIRECTORY}"
  "${SHADER_DIRECTORY}header/"
  "${SHADER_DIRECTORY}header/aabb/"
  "${SHADER_DIRECTORY}header/frustum_culling/"
  "${SHADER_DIRECTORY}header/graphics/"
  "${SHADER_DIRECTORY}header/physics/"
  "${SHADER_DIRECTORY}header/sphere/"
  "${SHADER_DIRECTORY}spec_constants/"
  )

set(ECS_OUT_FILE
  "${RESOURCE_DIRECTORY}ecs_resources.hpp"
)

set(AUDIO_OUT_FILE
  "${RESOURCE_DIRECTORY}audio_resources.hpp"
)

set(MESH_OUT_FILE
  "${RESOURCE_DIRECTORY}mesh_resources.hpp"
)

set(SHADER_REFLECT_OUT_FILE
  "${RESOURCE_DIRECTORY}shader_resources.hpp"
)

set(SPIRV_OUT_DIR
  "${RESOURCE_DIRECTORY}spirv/"
)

set(SHADER_SOURCE_FILES
  #"${SHADER_DIRECTORY}src/*.comp"
  "${SHADER_DIRECTORY}src/basic_shader.vert"
  "${SHADER_DIRECTORY}src/basic_shader.frag"
  #"${SHADER_DIRECTORY}*.glsl"
  #"${SHADER_DIRECTORY}src/aabb_render.vert"
  #"${SHADER_DIRECTORY}aabb/aabb_render.frag"
  #"${SHADER_DIRECTORY}ibl/brdf_lut.comp"
  #"${SHADER_DIRECTORY}clustered_forward/light_culling.comp"
  #"${SHADER_DIRECTORY}clustered_forward/light_culling_batch.comp"
  #"${SHADER_DIRECTORY}common/light_orb.vert"
  #"${SHADER_DIRECTORY}common/light_orb.frag"
  #"${SHADER_DIRECTORY}bindless/scene.vert"
  #"${SHADER_DIRECTORY}clustered_forward/scene.frag"
)

set(AUDIO_SAMPLES 
  "${PREBUILD_DIRECTORY}in/sample.wav"
)

set(OBJ_MESH_FILES 
  "${PREBUILD_DIRECTORY}in/bottom_alley_angle.obj"
)

set(ECS_FILES
  "${PREBUILD_DIRECTORY}in/ecs_typedef.csv"
  "${PREBUILD_DIRECTORY}in/ecs_components.csv"
  "${PREBUILD_DIRECTORY}in/ecs_archetypes.csv"
)

set(GLSL_FILES 
  ${SHADER_DIRECTORY}
  )

set(COMPILE_TIME_TEST_FLAGS "-pg -v -ftime-report")
set(HEAP_SIZE_FLAGS "--param gcc-min-expand=0")

#set(CMAKE_CXX_FLAGS "${COMPILE_TIME_TEST_FLAGS}")
#set(CMAKE_EXE_LINKER_FLAGS "${COMPILE_TIME_TEST_FLAGS}")

add_subdirectory(lofi)
add_subdirectory(external)
#add_subdirectory(prebuild)

#set(LUA_INCLUDE "${CMAKE_CURRENT_SOURCE_DIR}/external/lua/include/")


compile_prebuild_executables()

#set these to OFF if you want to recompile the resource files
set(GRAPHICS_RESOURCES_UP_TO_DATE OFF CACHE BOOL "forcing graphics resources to update" FORCE)
set(AUDIO_RESOURCES_UP_TO_DATE OFF CACHE BOOL "forcing audio resources to update" FORCE)
set(ECS_RESOURCES_UP_TO_DATE OFF CACHE BOOL "forcing ecs resources to update" FORCE)
set(SHADER_RESOURCES_UP_TO_DATE OFF CACHE BOOL "forcing shader resources to update" FORCE)

update_audio_config()
update_graphics_config()
update_ecs_config()
compile_shaders()
update_shader_config()

add_subdirectory(roxi)
add_subdirectory(test)
add_subdirectory(projects)

