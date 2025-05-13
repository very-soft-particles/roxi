macro(compile_prebuild_executables)
  execute_process(COMMAND "cmake" "-S" "./prebuild/" "-G" "Ninja" "-D" "CMAKE_EXPORT_COMPILE_COMMANDS=ON"
    "-B" "./prebuild/ninja-build/")
  execute_process(COMMAND "cmake" "--build" "./prebuild/ninja-build/")
endmacro(compile_prebuild_executables)
