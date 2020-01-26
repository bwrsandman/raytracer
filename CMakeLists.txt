cmake_minimum_required (VERSION 3.10)
project (Raytracer)

find_program(GLSLVALIDATOR glslangValidator)

file(GLOB sources CONFIGURE_DEPENDS src/*.cpp src/materials/*.cpp src/hittable/*.cpp)
file(GLOB_RECURSE headers CONFIGURE_DEPENDS include/*.h)
file(GLOB_RECURSE private_impl_sources CONFIGURE_DEPENDS src/private_impl/*)

file(GLOB shaders CONFIGURE_DEPENDS src/shaders/*.glsl)
file(GLOB shader_headers CONFIGURE_DEPENDS src/shaders/*.h)

set(assets
  ${CMAKE_CURRENT_SOURCE_DIR}/assets/earth/earth_albedo.jpg
  ${CMAKE_CURRENT_SOURCE_DIR}/assets/earth/earth_normal_map.tga
  # ${CMAKE_CURRENT_SOURCE_DIR}/assets/whitted/whitted_floor.png
  # ${CMAKE_CURRENT_SOURCE_DIR}/assets/models/BoxTextured.gltf
  # ${CMAKE_CURRENT_SOURCE_DIR}/assets/models/DamagedHelmet.gltf
  ${CMAKE_CURRENT_SOURCE_DIR}/assets/models/Duck.gltf
  # ${CMAKE_CURRENT_SOURCE_DIR}/assets/models/Sponza.gltf
)

# Tell CMake how to compile shaders into SPIV, then into headers
foreach(shader ${shaders})
    file(RELATIVE_PATH shader_relative_path "${CMAKE_CURRENT_SOURCE_DIR}" "${shader}")
    get_filename_component(shader_relative_directory ${shader_relative_path} DIRECTORY)
    get_filename_component(shader_name ${shader_relative_path} NAME_WE)
    set(compiled_shader ${CMAKE_CURRENT_BINARY_DIR}/${shader_relative_directory}/${shader_name}.h)
    add_custom_command(
        OUTPUT ${compiled_shader}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/${shader_relative_directory}
        COMMAND
            ${GLSLVALIDATOR}
            --target-env opengl
            -o ${compiled_shader}
            --vn ${shader_name}
            "$<$<CONFIG:debug>:-g -Od>$<$<CONFIG:relwithdebinfo>:-g>$<$<CONFIG:minsizerel>:-Os>"
            ${shader}
        DEPENDS ${shader} ${shader_headers}
    )
    list(APPEND sources ${compiled_shader})
endforeach()

# Tell CMake how to include assets
foreach(asset ${assets})
    get_filename_component(asset_name ${asset} NAME)
    set(asset_output ${CMAKE_CURRENT_BINARY_DIR}/${asset_name})
    add_custom_command(
            OUTPUT ${asset_output}
            COMMAND ${CMAKE_COMMAND} -E copy ${asset} ${asset_output}
            DEPENDS ${asset}
    )
    list(APPEND sources ${asset_output})
    set(EMBED_FILES "${EMBED_FILES} --embed-file ${asset_name}")
endforeach()

add_library(RaytracerLib STATIC ${sources} ${headers} ${private_impl_sources})
add_executable(Raytracer app/main.cpp)

if (EMSCRIPTEN)
    set_target_properties(RaytracerLib PROPERTIES
      COMPILE_FLAGS_DEBUG "-g4"
      LINK_FLAGS_DEBUG "-g4"
      COMPILE_FLAGS "-s USE_SDL=2 -s USE_PTHREADS=1"
      LINK_FLAGS "-s USE_SDL=2 -s FULL_ES3=1 -s GL_ASSERTIONS=1 -s USE_WEBGL2=1 -s USE_PTHREADS=1 -s TOTAL_MEMORY=128MB ${EMBED_FILES} --emrun"
      SUFFIX ".html"
    )
    set_target_properties(Raytracer PROPERTIES
      COMPILE_FLAGS_DEBUG "-g4"
      LINK_FLAGS_DEBUG "-g4"
      COMPILE_FLAGS "-s USE_SDL=2 -s USE_PTHREADS=1"
      LINK_FLAGS "-s USE_SDL=2 -s FULL_ES3=1 -s GL_ASSERTIONS=1 -s USE_WEBGL2=1 -s USE_PTHREADS=1 -s TOTAL_MEMORY=128MB ${EMBED_FILES} --emrun"
      SUFFIX ".html"
    )
    list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules/backport")
    include(FetchContent)
    FetchContent_Declare(
      spirv-cross
      URL https://github.com/KhronosGroup/SPIRV-Cross/archive/2019-11-01.tar.gz
      URL_HASH MD5=841d5055ef364a4dea7b06378336212a
      CMAKE_CACHE_ARGS "-DSPIRV_CROSS_EXCEPTIONS_TO_ASSERTIONS:STRING=ON -DSPIRV_CROSS_CLI:STRING=OFF -DSPIRV_CROSS_ENABLE_TESTS:STRING=OFF"
    )
    FetchContent_MakeAvailable(spirv-cross)
    set_target_properties(spirv-cross-glsl PROPERTIES
      COMPILE_FLAGS_DEBUG "-g4"
      LINK_FLAGS_DEBUG "-g4"
      COMPILE_FLAGS "-s USE_PTHREADS=1"
      LINK_FLAGS "-s USE_PTHREADS=1")
    target_compile_definitions(spirv-cross-glsl INTERFACE SPIRV_CROSS_EXCEPTIONS_TO_ASSERTIONS)
    set(LIBRARIES glad imgui spirv-cross-glsl)
else()
    find_package(SDL2 REQUIRED)
    find_package(spirv_cross_core REQUIRED)
    find_package(spirv_cross_glsl REQUIRED)
    set(LIBRARIES SDL2::SDL2 glad imgui spirv-cross-core spirv-cross-glsl)
endif()

# 3d connexion 3d mouse
find_library(SPNAV_LIBRARY spnav)
if (SPNAV_LIBRARY)
    target_compile_definitions(RaytracerLib PRIVATE USE_SPNAV=1)
    target_link_libraries(RaytracerLib PRIVATE ${SPNAV_LIBRARY})
endif()

add_subdirectory(3rd_party/glad)
add_subdirectory(3rd_party/imgui-1.73)

target_include_directories(RaytracerLib
  PUBLIC
    include/
  PRIVATE
    3rd_party/stb
    3rd_party/nlohmann-json
    3rd_party/tinygltf
    # Compiled shaders
    ${CMAKE_CURRENT_BINARY_DIR}/src)
target_link_libraries(RaytracerLib PRIVATE ${LIBRARIES})
target_compile_definitions(RaytracerLib PRIVATE SDL_MAIN_HANDLED)
if (MSVC)
  target_compile_options(RaytracerLib
    PUBLIC
      /arch:AVX2
    PRIVATE
      /W4
      /wd4505 # disable unused function
      /wd4324 # structure padded due to alignment specifer
      #/WX  # Warning are errors
  )
elseif(EMSCRIPTEN)
  target_compile_options(RaytracerLib
    PUBLIC
      -msimd128
    PRIVATE
      -fno-exceptions
      -Wall
      -Wextra
      -Wno-unused-function
      -Wno-format-nonliteral
      -Wno-dollar-in-identifier-extension
      -Wno-gnu-zero-variadic-macro-arguments
      -Werror
      -pedantic
      -pedantic-errors
  )
else()
  target_compile_options(RaytracerLib
    PUBLIC
      -march=haswell
    PRIVATE
      -fno-exceptions
      -Wall
      -Wextra
      -Wno-unused-function
      -Wno-format-nonliteral
      -Werror
      -pedantic
      -pedantic-errors

      -Wcomment
      -Wimplicit
      -Wimport
      -Winline
      -Wparentheses
      -Wreturn-type
      -Wstack-protector
      -Wstrict-aliasing
      -Wstrict-aliasing=2
      -Wswitch
      -Wswitch-default
      -Wswitch-enum
      -Wuninitialized
      -Wmissing-braces
      -Wmissing-field-initializers
      -Wmissing-format-attribute
      -Wmissing-include-dirs
      -Wmissing-noreturn
      -Wpointer-arith
      -Wredundant-decls
      -Wsequence-point
      -Wshadow
      -Wsign-compare
      -Wtrigraphs
      -Wunknown-pragmas
      -Wunreachable-code
      -Wunused-label
      -Wunused-parameter
      -Wunused-value
      -Wunused-variable
      -Wvariadic-macros
      -Wvolatile-register-var
      -Wwrite-strings
      -Wdisabled-optimization
      -Winit-self
      -Winvalid-pch
      -Wlong-long
      -Weffc++
      -Waggregate-return
      -Wchar-subscripts

      # -Wcast-align
      # -Wconversion
      # -Wpacked
      # -Wpadded
      # -Wcast-qual
      # -Wformat
      # -Wformat=2
      # -Wformat-security
      # -Wformat-y2k
  )
endif()
set_property(TARGET RaytracerLib PROPERTY CXX_STANDARD 17)

target_link_libraries(Raytracer PRIVATE RaytracerLib)

add_subdirectory(benchmarks)
