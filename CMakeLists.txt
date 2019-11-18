cmake_minimum_required (VERSION 3.10)
project (Whitted_Raytracing)

find_package(SDL2 REQUIRED)
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

add_subdirectory(3rd_party/glad)

target_include_directories(Whitted_Raytracing
  PRIVATE
    include/
    # Compiled shaders
    ${CMAKE_CURRENT_BINARY_DIR}/src)
target_link_libraries(Whitted_Raytracing PRIVATE SDL2::SDL2 glad)
target_compile_definitions(Whitted_Raytracing PRIVATE SDL_MAIN_HANDLED)
