
macro(generate_test TEST_NAME)
  add_executable(${TEST_NAME} "${TEST_NAME}.cpp")
  target_include_directories(${TEST_NAME} PUBLIC ${LOFI_INCLUDE_PATH})
  target_link_libraries(${TEST_NAME} PUBLIC LOFI)
endmacro()
  
