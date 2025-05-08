cmake_minimum_required(VERSION 3.23)

project(LOFI)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "-static-libstdc++")

add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/core")

add_library(
  ${PROJECT_NAME}
    INTERFACE
)

set(LOFI_INCLUDE_PATH "${LOFI_INTERNAL_INCLUDE_PATH}")
set(LOFI_PATH "${CMAKE_CURRENT_LIST_DIR}/")

target_include_directories(
  ${PROJECT_NAME}
    PUBLIC
    ${LOFI_INTERNAL_INCLUDE_PATH}
)

if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif()

if(NOT DEFINED LOFI_NO_TESTS)
  set(LOFI_BUILD_TESTS)
else()
endif()

if(DEFINED LOFI_BUILD_TESTS)
  add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/test")
endif()
