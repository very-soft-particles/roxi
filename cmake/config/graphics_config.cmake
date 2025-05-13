
macro(update_graphics_config)

  if(NOT GRAPHICS_RESOURCES_UP_TO_DATE)
    message("graphics not up to date, running function")
    execute_process(COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/prebuild/ninja-build/graphics_config.exe"
      ${OBJ_MESH_FILES}
      ${MESH_OUT_FILE}
    )
  endif()

  set(GRAPHICS_RESOURCES_UP_TO_DATE ON CACHE BOOL "are graphics resources compiled?" FORCE)

endmacro(update_graphics_config)
