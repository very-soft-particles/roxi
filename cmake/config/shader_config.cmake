
macro(update_shader_config)

  if(NOT SHADER_RESOURCES_UP_TO_DATE)
    message("shader reflect not up to date, running function spirv dir: ${SPIRV_OUT_DIR} out file:
    ${SHADER_REFLECT_OUT_FILE} process source ${CMAKE_CURRENT_SOURCE_DIR}/prebuild/ninja-build/shader_config.exe
")
    execute_process(COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/prebuild/ninja-build/shader_config.exe"
      ${SPIRV_OUT_DIR}
      ${SHADER_REFLECT_OUT_FILE}
    )
  endif()

  set(SHADER_RESOURCES_UP_TO_DATE ON CACHE BOOL "are shader resources compiled?" FORCE)

endmacro(update_shader_config)
