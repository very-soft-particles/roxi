macro(compile_shaders) 

  find_package(Vulkan REQUIRED)
  # try reversing all slashes on windows? must be a better way...

  list(LENGTH SHADER_SOURCE_FILES FILE_COUNT)
  if(FILE_COUNT EQUAL 0)
    message(COMMENT "zero shaders passed to compile shaders")
  endif()

  set(SHADER_COMMANDS)
  set(SHADER_PRODUCTS)

  foreach(SHADER_SOURCE IN LISTS SHADER_SOURCE_FILES)
    cmake_path(ABSOLUTE_PATH SHADER_SOURCE NORMALIZE)
    cmake_path(GET SHADER_SOURCE FILENAME SHADER_NAME)

    list(APPEND SHADER_COMMANDS COMMAND)
    #list(APPEND SHADER_COMMANDS Vulkan::glslc)
    list(APPEND SHADER_COMMANDS Vulkan::glslc)
    list(APPEND SHADER_COMMANDS "-std=460core")
    list(APPEND SHADER_COMMANDS "${SHADER_SOURCE}")
    foreach(INCLUDE_DIR IN LISTS SHADER_INCLUDE_DIRECTORIES)
      list(APPEND SHADER_COMMANDS "-I")
      list(APPEND SHADER_COMMANDS "${INCLUDE_DIR}")
    endforeach()
    list(APPEND SHADER_COMMANDS "-o")
    list(APPEND SHADER_COMMANDS "${SPIRV_OUT_DIR}${SHADER_NAME}.spv")
  endforeach()

  add_custom_target(${PROJECT_NAME} ALL
    ${SHADER_COMMANDS}
    COMMENT "compiling shaders ${PROJECT_NAME}"
    SOURCES ${SHADER_SOURCE_FILES}
    BYPRODUCTS ${SHADER_PRODUCTS}
  )
endmacro(compile_shaders)
