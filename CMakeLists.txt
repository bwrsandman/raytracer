cmake_minimum_required (VERSION 3.10)
project (Whitted_Raytracing)

file(GLOB sources CONFIGURE_DEPENDS src/*.cpp)
file(GLOB headers CONFIGURE_DEPENDS include/*.h)
file(GLOB_RECURSE private_impl_sources CONFIGURE_DEPENDS src/private_impl/*)

add_executable(Whitted_Raytracing ${sources} ${headers} ${private_impl_sources})

find_package(SDL2 REQUIRED)

add_subdirectory(3rd_party/glad)

target_include_directories(Whitted_Raytracing PRIVATE include/)
target_link_libraries(Whitted_Raytracing PRIVATE SDL2::SDL2 glad)
target_compile_definitions(Whitted_Raytracing PRIVATE SDL_MAIN_HANDLED)
