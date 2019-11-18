cmake_minimum_required (VERSION 3.10)
project (Whitted_Raytracing)

file(GLOB sources CONFIGURE_DEPENDS src/*.cpp)
file(GLOB headers CONFIGURE_DEPENDS include/*.h)

add_executable(Whitted_Raytracing ${sources} ${headers} )

set(OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL REQUIRED)
find_package(SDL2 REQUIRED)

target_include_directories(Whitted_Raytracing PRIVATE include/)
target_link_libraries(Whitted_Raytracing PRIVATE SDL2::SDL2 OpenGL::GL)
target_compile_definitions(Whitted_Raytracing PRIVATE SDL_MAIN_HANDLED GL_GLEXT_PROTOTYPES)
