
macro(update_ecs_config)

  if(NOT ECS_RESOURCES_UP_TO_DATE)
    message("ecs not up to date, running function ")
    execute_process(COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/prebuild/ninja-build/ecs_config.exe"
      ${ECS_FILES}
      ${ECS_OUT_FILE}
    )
  endif()

  set(ECS_RESOURCES_UP_TO_DATE ON CACHE BOOL "are ecs resources compiled?" FORCE)

endmacro(update_ecs_config)
