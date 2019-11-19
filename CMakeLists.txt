cmake_minimum_required (VERSION 3.10)
project (Whitted_Raytracing)

find_program(GLSLVALIDATOR glslangValidator)

file(GLOB sources CONFIGURE_DEPENDS src/*.cpp)
file(GLOB headers CONFIGURE_DEPENDS include/*.h)
file(GLOB_RECURSE private_impl_sources CONFIGURE_DEPENDS src/private_impl/*)

file(GLOB shaders CONFIGURE_DEPENDS src/shaders/*.glsl)

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
        DEPENDS ${shader}
    )
    list(APPEND sources ${compiled_shader})
endforeach()

add_executable(Whitted_Raytracing ${sources} ${headers}  ${private_impl_sources})

if (EMSCRIPTEN)
    set_target_properties(Whitted_Raytracing PROPERTIES
      COMPILE_FLAGS "-s MAIN_MODULE=1"
      LINK_FLAGS "-s USE_SDL=2 -s MAIN_MODULE=1 -s FULL_ES3=1 -s GL_ASSERTIONS=1 -s USE_WEBGL2=1 --emrun"
      POSITION_INDEPENDENT_CODE ON
      SUFFIX ".html"
    )
    list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules/backport")
    include(FetchContent)
    FetchContent_Declare(
      spirv-cross
      URL https://github.com/KhronosGroup/SPIRV-Cross/archive/2019-11-01.tar.gz
      URL_HASH MD5=841d5055ef364a4dea7b06378336212a
    )
    FetchContent_MakeAvailable(spirv-cross)
    set_target_properties(spirv-cross-core PROPERTIES
      COMPILE_FLAGS "-s SIDE_MODULE=1"
      LINK_FLAGS "-s SIDE_MODULE=1"
      POSITION_INDEPENDENT_CODE ON
    )
    set_target_properties(spirv-cross-glsl PROPERTIES
      COMPILE_FLAGS "-s SIDE_MODULE=1"
      LINK_FLAGS "-s SIDE_MODULE=1"
      POSITION_INDEPENDENT_CODE ON
    )
    set(LIBRARIES glad imgui spirv-cross-glsl)
else()
    find_package(SDL2 REQUIRED)
    find_package(spirv_cross_core REQUIRED)
    find_package(spirv_cross_glsl REQUIRED)
    set(LIBRARIES SDL2::SDL2 glad imgui spirv-cross-core spirv-cross-glsl)
endif()

add_subdirectory(3rd_party/glad)
add_subdirectory(3rd_party/imgui-1.73)

target_include_directories(Whitted_Raytracing
  PRIVATE
    include/
    # Compiled shaders
    ${CMAKE_CURRENT_BINARY_DIR}/src)
target_link_libraries(Whitted_Raytracing PRIVATE ${LIBRARIES})
target_compile_definitions(Whitted_Raytracing PRIVATE SDL_MAIN_HANDLED)
set_property(TARGET Whitted_Raytracing PROPERTY CXX_STANDARD 17)
