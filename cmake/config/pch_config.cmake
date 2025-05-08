
macro(update_pch_config)

  target_precompile_headers(${PROJECT_NAME}
    PRIVATE
    ${PRIVATE_PRECOMPILED_HEADERS}
    PUBLIC
    ${PUBLIC_PRECOMPILED_HEADERS}
    INTERFACE
    ${INTERFACE_PRECOMPILED_HEADERS}
  )

endmacro(update_pch_config)
