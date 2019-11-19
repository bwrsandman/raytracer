add_library(glad STATIC src/glad.c)
target_include_directories(glad PUBLIC include/)
if (EMSCRIPTEN)
  set_target_properties(glad PROPERTIES
    COMPILE_FLAGS "-s SIDE_MODULE=1"
    POSITION_INDEPENDENT_CODE ON
  )
endif()
