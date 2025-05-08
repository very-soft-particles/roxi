macro(update_audio_config)

  if(NOT AUDIO_RESOURCES_UP_TO_DATE)
    message("audio not up to date, running function ")
    execute_process(COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/ninja-build/prebuild/audio_config.exe"
      ${AUDIO_SAMPLES}
      ${AUDIO_OUT_FILE}
    )
  endif()

  set(AUDIO_RESOURCES_UP_TO_DATE ON CACHE BOOL "are audio resources compiled?" FORCE)

endmacro(update_audio_config)
