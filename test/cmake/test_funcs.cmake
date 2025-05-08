
macro(generate_test TEST_NAME)
  add_executable(${TEST_NAME} "${TEST_NAME}.cpp")
  target_include_directories(${TEST_NAME} PUBLIC ${ROXI_INCLUDE_PATHS} ${ROXI_INTERNAL_INCLUDE_PATHS})
  target_link_libraries(${TEST_NAME} PUBLIC roxi
    PRIVATE Vulkan::Vulkan)
endmacro()
  
